"""Shared types, parsing, and code generation for metal-cpp tooling.

Contains ObjC header parsing (libclang), type resolution, data classes,
and C++ code generation following the metal-cpp patterns used by Apple.

Used by both generate.py (full framework generation) and patch_metalcpp.py
(targeted amendments to upstream metal-cpp).
"""

from __future__ import annotations

import logging
import re
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional

from clang.cindex import CursorKind, Index, TranslationUnit

log = logging.getLogger(__name__)

# ── Built-in type map (types already in metal-cpp) ───────────────────────

BUILTIN_TYPE_MAP: dict[str, str] = {
    # Foundation
    "NSObject": "NS::Object",
    "NSString": "NS::String",
    "NSError": "NS::Error",
    "NSArray": "NS::Array",
    "NSDictionary": "NS::Dictionary",
    "NSNumber": "NS::Number",
    "NSURL": "NS::URL",
    "NSBundle": "NS::Bundle",
    "NSData": "NS::Data",
    "NSValue": "NS::Value",
    "NSSet": "NS::Set",
    "NSUInteger": "NS::UInteger",
    "NSInteger": "NS::Integer",
    "NSTimeInterval": "NS::TimeInterval",
    "BOOL": "bool",
    "NSRect": "CGRect",
    "NSSize": "CGSize",
    "NSPoint": "CGPoint",
    # Metal
    "MTLDevice": "MTL::Device",
    "MTLTexture": "MTL::Texture",
    "MTLBuffer": "MTL::Buffer",
    "MTLLibrary": "MTL::Library",
    "MTLCommandQueue": "MTL::CommandQueue",
    "MTLCommandBuffer": "MTL::CommandBuffer",
    "MTLRenderPipelineState": "MTL::RenderPipelineState",
    "MTLComputePipelineState": "MTL::ComputePipelineState",
    "MTLPixelFormat": "MTL::PixelFormat",
    "MTLResourceOptions": "MTL::ResourceOptions",
    "MTLResidencySet": "MTL::ResidencySet",
    # QuartzCore
    "CAMetalLayer": "CA::MetalLayer",
    "CAMetalDrawable": "CA::MetalDrawable",
}

# Types that pass through unchanged (C, CoreFoundation, etc.)
PASSTHROUGH_TYPES: set[str] = {
    "void", "bool", "float", "double",
    "char", "short", "int", "long",
    "unsigned char", "unsigned short", "unsigned int", "unsigned long",
    "long long", "unsigned long long",
    "int8_t", "int16_t", "int32_t", "int64_t",
    "uint8_t", "uint16_t", "uint32_t", "uint64_t",
    "size_t", "ssize_t", "ptrdiff_t",
    "CGRect", "CGSize", "CGPoint", "CGFloat",
    "CGColorSpaceRef", "CGDirectDisplayID", "CGColorRef",
    "CGAffineTransform",
    "CFStringRef", "CFTypeRef", "CFTimeInterval",
    "dispatch_queue_t", "dispatch_data_t",
    "IOSurfaceRef",
    "SEL", "Class",
}

# Resolved type → system header needed
SYSTEM_HEADER_FOR_TYPE: dict[str, str] = {
    "CGRect": "CoreGraphics/CoreGraphics.h",
    "CGSize": "CoreGraphics/CoreGraphics.h",
    "CGPoint": "CoreGraphics/CoreGraphics.h",
    "CGFloat": "CoreGraphics/CoreGraphics.h",
    "CGColorSpaceRef": "CoreGraphics/CoreGraphics.h",
    "CGDirectDisplayID": "CoreGraphics/CoreGraphics.h",
    "CGColorRef": "CoreGraphics/CoreGraphics.h",
    "CGAffineTransform": "CoreGraphics/CoreGraphics.h",
}


# ── Data classes ──────────────────────────────────────────────────────────

@dataclass
class ObjCProperty:
    name: str
    objc_type: str
    is_readonly: bool
    is_class_property: bool = False


@dataclass
class ObjCParam:
    name: str
    objc_type: str


@dataclass
class ObjCMethod:
    selector: str
    return_type: str
    params: list[ObjCParam] = field(default_factory=list)
    is_class_method: bool = False

    @property
    def sel_accessor(self) -> str:
        """Selector accessor for _PRIVATE_DEF_SEL: colons replaced with underscores."""
        return self.selector.replace(":", "_")

    @property
    def cpp_name(self) -> str:
        """C++ method name: first selector piece (without colon)."""
        return self.selector.split(":")[0]


@dataclass
class ObjCEnumValue:
    name: str
    value: Optional[int]


@dataclass
class ObjCEnum:
    name: str
    underlying_type: str
    values: list[ObjCEnumValue] = field(default_factory=list)


@dataclass
class ObjCClass:
    name: str
    superclass: str = "NSObject"
    properties: list[ObjCProperty] = field(default_factory=list)
    methods: list[ObjCMethod] = field(default_factory=list)
    protocols: list[str] = field(default_factory=list)


@dataclass
class FrameworkData:
    """Parsed data for one framework from one SDK."""
    classes: list[ObjCClass] = field(default_factory=list)
    enums: list[ObjCEnum] = field(default_factory=list)


# ── Type resolution ───────────────────────────────────────────────────────

class TypeResolver:
    """Maps ObjC type spellings to C++ types."""

    def __init__(self) -> None:
        self.type_map: dict[str, str] = dict(BUILTIN_TYPE_MAP)
        self.unresolved: list[tuple[str, str]] = []  # (type, context)

    def register(self, objc_name: str, cpp_type: str) -> None:
        """Register a generated type mapping."""
        self.type_map[objc_name] = cpp_type

    def resolve(self, objc_type: str, context: str = "") -> str:
        """Resolve an ObjC type string to its C++ equivalent.

        Returns 'void*' and logs a warning for unresolvable types.
        Returns '__instancetype__' for instancetype (caller handles).
        """
        s = self._normalize(objc_type)

        # instancetype → handled by caller with the concrete class
        if s == "instancetype":
            return "__instancetype__"

        # id<Protocol>
        m = re.match(r"^id<(\w+)>$", s)
        if m:
            proto = m.group(1)
            cpp = self.type_map.get(proto)
            if cpp:
                return f"{cpp}*"
            return self._unresolved(objc_type, context)

        # Plain id
        if s == "id":
            return "void*"

        # ObjC object pointer: ClassName *
        m = re.match(r"^(\w+)\s*\*$", s)
        if m:
            cls = m.group(1)
            cpp = self.type_map.get(cls)
            if cpp:
                return f"{cpp}*"
            if cls in PASSTHROUGH_TYPES or cls in ("void", "char", "unsigned"):
                return s
            return self._unresolved(objc_type, context)

        # Direct lookup (typedef or basic type)
        if s in self.type_map:
            return self.type_map[s]
        if s in PASSTHROUGH_TYPES:
            return s

        # Pointer to passthrough: void *, const char *, etc.
        if s.endswith("*"):
            base = s[:-1].strip()
            if base in PASSTHROUGH_TYPES or base.startswith("const"):
                return s

        # Enum types that resolve to their underlying type through the map
        # (e.g., NSWindowStyleMask if registered)
        # Fall through to unresolved if nothing matches.
        return self._unresolved(objc_type, context)

    def _normalize(self, s: str) -> str:
        """Strip nullability/ownership qualifiers and extra whitespace."""
        s = re.sub(
            r"\b(__nullable|__nonnull|_Nullable|_Nonnull|"
            r"__kindof|__autoreleasing|__unsafe_unretained|__weak|__strong)\b",
            "", s,
        )
        return re.sub(r"\s+", " ", s).strip()

    def _unresolved(self, objc_type: str, context: str) -> str:
        log.warning("Unresolvable type: '%s' (in %s)", objc_type, context or "?")
        self.unresolved.append((objc_type, context))
        return "void*"


# ── ObjC header parsing ──────────────────────────────────────────────────

def _safe_kind(cursor) -> Optional[CursorKind]:
    """Get cursor kind, returning None for unknown kinds from newer SDKs."""
    try:
        return cursor.kind
    except ValueError:
        return None


class ObjCParser:
    """Parse Objective-C headers using libclang."""

    def __init__(self, sdk_path: Path) -> None:
        self.sdk_path = sdk_path
        self.index = Index.create()

    def parse_header(self, header_path: Path) -> FrameworkData:
        """Parse a single ObjC header and extract classes/enums."""
        args = [
            "-x", "objective-c",
            "-isysroot", str(self.sdk_path),
            "-fno-objc-arc",
            "-Wno-everything",
        ]
        tu = self.index.parse(
            str(header_path), args=args,
            options=(TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD
                     | TranslationUnit.PARSE_SKIP_FUNCTION_BODIES),
        )

        data = FrameworkData()
        target = str(header_path)

        # First pass: collect @interface declarations
        classes_by_name: dict[str, ObjCClass] = {}
        for cursor in tu.cursor.get_children():
            loc = cursor.location
            if not loc.file or loc.file.name != target:
                continue
            kind = _safe_kind(cursor)
            if kind == CursorKind.OBJC_INTERFACE_DECL:
                cls = self._parse_class(cursor)
                if cls:
                    classes_by_name[cls.name] = cls
            elif kind == CursorKind.ENUM_DECL:
                enum = self._parse_enum(cursor)
                if enum:
                    data.enums.append(enum)

        # Second pass: merge members from categories/extensions into classes
        for cursor in tu.cursor.get_children():
            loc = cursor.location
            if not loc.file or loc.file.name != target:
                continue
            if _safe_kind(cursor) != CursorKind.OBJC_CATEGORY_DECL:
                continue
            # Find which class this category extends
            class_name = None
            for child in cursor.get_children():
                if _safe_kind(child) == CursorKind.OBJC_CLASS_REF:
                    class_name = child.spelling
                    break
            if not class_name or class_name not in classes_by_name:
                continue
            # Parse the category like a class and merge members
            cat = self._parse_class_members(cursor)
            target_cls = classes_by_name[class_name]
            existing_props = {p.name for p in target_cls.properties}
            existing_sels = {m.selector for m in target_cls.methods}
            for p in cat.properties:
                if p.name not in existing_props:
                    target_cls.properties.append(p)
            for m in cat.methods:
                if m.selector not in existing_sels:
                    target_cls.methods.append(m)

        data.classes = list(classes_by_name.values())
        return data

    def _parse_class_members(self, cursor) -> ObjCClass:
        """Parse properties/methods from a cursor (class or category)."""
        cls = ObjCClass(name="")
        for child in cursor.get_children():
            kind = _safe_kind(child)
            if kind == CursorKind.OBJC_PROPERTY_DECL:
                prop = self._parse_property(child)
                if prop:
                    cls.properties.append(prop)
            elif kind == CursorKind.OBJC_INSTANCE_METHOD_DECL:
                method = self._parse_method(child, is_class=False)
                if method:
                    cls.methods.append(method)
            elif kind == CursorKind.OBJC_CLASS_METHOD_DECL:
                method = self._parse_method(child, is_class=True)
                if method:
                    cls.methods.append(method)
            elif kind == CursorKind.OBJC_PROTOCOL_REF:
                cls.protocols.append(child.spelling)
        # Remove synthesized accessor methods that duplicate properties.
        prop_sels = set()
        for prop in cls.properties:
            prop_sels.add(prop.name)
            if not prop.is_readonly:
                prop_sels.add(f"set{prop.name[0].upper()}{prop.name[1:]}:")
        cls.methods = [m for m in cls.methods if m.selector not in prop_sels]
        return cls

    def _parse_class(self, cursor) -> Optional[ObjCClass]:
        name = cursor.spelling
        if not name:
            return None

        superclass = ""
        for child in cursor.get_children():
            if _safe_kind(child) == CursorKind.OBJC_SUPER_CLASS_REF:
                superclass = child.spelling
                break

        cls = self._parse_class_members(cursor)
        cls.name = name
        cls.superclass = superclass or "NSObject"
        return cls

    def _parse_property(self, cursor) -> Optional[ObjCProperty]:
        name = cursor.spelling
        if not name:
            return None

        objc_type = cursor.type.spelling
        tokens = [t.spelling for t in cursor.get_tokens()]
        is_readonly = "readonly" in tokens
        is_class = "class" in tokens

        return ObjCProperty(
            name=name,
            objc_type=objc_type,
            is_readonly=is_readonly,
            is_class_property=is_class,
        )

    def _parse_method(self, cursor, is_class: bool) -> Optional[ObjCMethod]:
        selector = cursor.spelling
        if not selector:
            return None

        return_type = cursor.result_type.spelling

        params = []
        for child in cursor.get_children():
            if _safe_kind(child) == CursorKind.PARM_DECL:
                params.append(ObjCParam(
                    name=child.spelling or f"param{len(params)}",
                    objc_type=child.type.spelling,
                ))

        return ObjCMethod(
            selector=selector,
            return_type=return_type,
            params=params,
            is_class_method=is_class,
        )

    def _parse_enum(self, cursor) -> Optional[ObjCEnum]:
        name = cursor.spelling
        if not name:
            return None

        underlying = cursor.enum_type.spelling if cursor.enum_type else "unsigned long"

        values = []
        for child in cursor.get_children():
            if _safe_kind(child) == CursorKind.ENUM_CONSTANT_DECL:
                values.append(ObjCEnumValue(
                    name=child.spelling,
                    value=child.enum_value,
                ))

        return ObjCEnum(name=name, underlying_type=underlying, values=values)


# ── Code generation ──────────────────────────────────────────────────────

class CodeGenerator:
    """Generate metal-cpp style C++ wrappers for a single framework."""

    def __init__(
        self,
        namespace: str,
        prefix: str,
        strip_prefix: str,
        resolver: TypeResolver,
        rel_to_root: str = "../../../",
    ) -> None:
        self.ns = namespace
        self.prefix = prefix
        self.strip_prefix = strip_prefix
        self.resolver = resolver
        self.rel_to_root = rel_to_root

        # Accumulated across all classes for Private.hpp
        self.all_selectors: dict[str, str] = {}  # accessor → ObjC selector string
        self.all_classes: set[str] = set()  # ObjC class names
        self.generated_headers: list[str] = []

    # ── Helpers ────────────────────────────────────────────────────────

    def cpp_class_name(self, objc_name: str) -> str:
        """Strip ObjC prefix to get C++ class name (e.g. NSScreen → Screen)."""
        if self.strip_prefix and objc_name.startswith(self.strip_prefix):
            stripped = objc_name[len(self.strip_prefix):]
            if stripped:
                return stripped
        return objc_name

    def _resolve(self, objc_type: str, cls_name: str = "", context: str = "") -> str:
        """Resolve type, handling instancetype → concrete class."""
        cpp = self.resolver.resolve(objc_type, context)
        if cpp == "__instancetype__" and cls_name:
            return f"{self.ns}::{self.cpp_class_name(cls_name)}*"
        return cpp

    def _setter_name(self, prop_name: str) -> str:
        return f"set{prop_name[0].upper()}{prop_name[1:]}"

    def _system_includes(self, resolved_types: set[str]) -> list[str]:
        """Determine system #include directives needed for the given resolved types."""
        headers = set()
        for t in resolved_types:
            # Strip pointer/const to get base type
            base = t.rstrip("*").strip()
            if base.startswith("const "):
                base = base[6:].strip()
            if base in SYSTEM_HEADER_FOR_TYPE:
                headers.add(SYSTEM_HEADER_FOR_TYPE[base])
        return sorted(headers)

    # ── Selector collection ───────────────────────────────────────────

    def collect_selectors(self, cls: ObjCClass) -> None:
        """Collect selector/class registrations for Private.hpp."""
        self.all_classes.add(cls.name)

        for prop in cls.properties:
            self.all_selectors[prop.name] = prop.name
            if not prop.is_readonly:
                sname = self._setter_name(prop.name)
                self.all_selectors[f"{sname}_"] = f"{sname}:"

        for method in cls.methods:
            self.all_selectors[method.sel_accessor] = method.selector

    # ── File generators ───────────────────────────────────────────────

    def generate_defines(self) -> str:
        p = self.prefix
        return "\n".join([
            "#pragma once",
            "",
            f'#include "{self.rel_to_root}Foundation/NSDefines.hpp"',
            "",
            f"#define _{p}_EXPORT _NS_EXPORT",
            f"#define _{p}_EXTERN _NS_EXTERN",
            f"#define _{p}_INLINE _NS_INLINE",
            f"#define _{p}_PACKED _NS_PACKED",
            "",
            f"#define _{p}_CONST(type, name) _NS_CONST(type, name)",
            f"#define _{p}_ENUM(type, name) _NS_ENUM(type, name)",
            f"#define _{p}_OPTIONS(type, name) _NS_OPTIONS(type, name)",
            "",
            f"#define _{p}_VALIDATE_SIZE(ns, name) _NS_VALIDATE_SIZE(ns, name)",
            f"#define _{p}_VALIDATE_ENUM(ns, name) _NS_VALIDATE_ENUM(ns, name)",
            "",
        ])

    def generate_private(self) -> str:
        p = self.prefix
        ns = self.ns
        lines = [
            "#pragma once",
            "",
            f'#include "{p}Defines.hpp"',
            "",
            "#include <objc/runtime.h>",
            "",
            f"#define _{p}_PRIVATE_CLS(symbol) (Private::Class::s_k##symbol)",
            f"#define _{p}_PRIVATE_SEL(accessor) (Private::Selector::s_k##accessor)",
            "",
            f"#if defined({p}_PRIVATE_IMPLEMENTATION)",
            "",
            "#ifdef METALCPP_SYMBOL_VISIBILITY_HIDDEN",
            f"#define _{p}_PRIVATE_VISIBILITY __attribute__((visibility(\"hidden\")))",
            "#else",
            f"#define _{p}_PRIVATE_VISIBILITY __attribute__((visibility(\"default\")))",
            "#endif // METALCPP_SYMBOL_VISIBILITY_HIDDEN",
            "",
            "#ifdef __OBJC__",
            f"#define _{p}_PRIVATE_OBJC_LOOKUP_CLASS(symbol) "
            "((__bridge void*)objc_lookUpClass(#symbol))",
            "#else",
            f"#define _{p}_PRIVATE_OBJC_LOOKUP_CLASS(symbol) "
            "objc_lookUpClass(#symbol)",
            "#endif // __OBJC__",
            "",
            f"#define _{p}_PRIVATE_DEF_CLS(symbol) "
            f"void* s_k##symbol _{p}_PRIVATE_VISIBILITY = "
            f"_{p}_PRIVATE_OBJC_LOOKUP_CLASS(symbol)",
            f"#define _{p}_PRIVATE_DEF_SEL(accessor, symbol) "
            f"SEL s_k##accessor _{p}_PRIVATE_VISIBILITY = sel_registerName(symbol)",
            "",
            "#else",
            "",
            f"#define _{p}_PRIVATE_DEF_CLS(symbol) extern void* s_k##symbol",
            f"#define _{p}_PRIVATE_DEF_SEL(accessor, symbol) "
            "extern SEL s_k##accessor",
            "",
            f"#endif // {p}_PRIVATE_IMPLEMENTATION",
            "",
        ]

        # Class registrations
        if self.all_classes:
            lines += [
                f"namespace {ns}",
                "{",
                "namespace Private",
                "{",
                "    namespace Class",
                "    {",
            ]
            for cls_name in sorted(self.all_classes):
                lines.append(f"        _{p}_PRIVATE_DEF_CLS({cls_name});")
            lines += [
                "    } // Class",
                "} // Private",
                f"}} // {ns}",
                "",
            ]

        # Selector registrations
        if self.all_selectors:
            lines += [
                f"namespace {ns}",
                "{",
                "namespace Private",
                "{",
                "    namespace Selector",
                "    {",
            ]
            for accessor in sorted(self.all_selectors):
                sel_str = self.all_selectors[accessor]
                lines.append(f"        _{p}_PRIVATE_DEF_SEL({accessor},")
                lines.append(f'            "{sel_str}");')
            lines += [
                "    } // Selector",
                "} // Private",
                f"}} // {ns}",
                "",
            ]

        return "\n".join(lines)

    def generate_class_header(self, cls: ObjCClass) -> str:
        p = self.prefix
        ns = self.ns
        cpp_name = self.cpp_class_name(cls.name)

        # Collect all resolved types to determine system includes
        resolved_types: set[str] = set()

        lines = [
            "#pragma once",
            "",
            f'#include "{p}Defines.hpp"',
            f'#include "{p}Private.hpp"',
            f'#include "{self.rel_to_root}Foundation/NSObject.hpp"',
        ]

        # Pre-resolve all types to determine system includes
        for prop in cls.properties:
            resolved_types.add(self._resolve(prop.objc_type, cls.name))
        for method in cls.methods:
            resolved_types.add(self._resolve(method.return_type, cls.name))
            for param in method.params:
                resolved_types.add(self._resolve(param.objc_type, cls.name))

        for header in self._system_includes(resolved_types):
            lines.append(f"#include <{header}>")

        lines += [
            "",
            f"namespace {ns}",
            "{",
            "",
            f"class {cpp_name} : public NS::Referencing<{cpp_name}>",
            "{",
            "public:",
        ]

        # Class properties (static getters)
        class_props = [p for p in cls.properties if p.is_class_property]
        instance_props = [p for p in cls.properties if not p.is_class_property]
        class_methods = [m for m in cls.methods if m.is_class_method]
        instance_methods = [m for m in cls.methods if not m.is_class_method]

        for prop in class_props:
            cpp_type = self._resolve(prop.objc_type, cls.name)
            lines.append(f"    static {cpp_type} {prop.name}();")

        for m in class_methods:
            ret = self._resolve(m.return_type, cls.name)
            params_str = self._fmt_params(m.params, cls.name)
            lines.append(f"    static {ret} {m.cpp_name}({params_str});")

        if class_props or class_methods:
            lines.append("")

        for prop in instance_props:
            cpp_type = self._resolve(prop.objc_type, cls.name)
            lines.append(f"    {cpp_type} {prop.name}() const;")
            if not prop.is_readonly:
                lines.append(
                    f"    void {self._setter_name(prop.name)}({cpp_type} {prop.name});")

        if instance_props:
            lines.append("")

        for m in instance_methods:
            ret = self._resolve(m.return_type, cls.name)
            params_str = self._fmt_params(m.params, cls.name)
            lines.append(f"    {ret} {m.cpp_name}({params_str}) const;")

        if instance_methods:
            lines.append("")

        lines += [
            "};",
            "",
            f"}} // namespace {ns}",
            "",
            "// --- Inline implementations ---",
            "",
        ]

        # ── Inline implementations ────────────────────────────────────

        # Class properties
        for prop in class_props:
            cpp_type = self._resolve(prop.objc_type, cls.name)
            sel = f"_{p}_PRIVATE_SEL({prop.name})"
            lines += [
                f"_{p}_INLINE {cpp_type} {ns}::{cpp_name}::{prop.name}()",
                "{",
                f"    return Object::sendMessage<{cpp_type}>("
                f"_{p}_PRIVATE_CLS({cls.name}), {sel});",
                "}",
                "",
            ]

        # Class methods
        for m in class_methods:
            ret = self._resolve(m.return_type, cls.name)
            params_str = self._fmt_params(m.params, cls.name)
            args = self._fmt_args(m.params)
            sel = f"_{p}_PRIVATE_SEL({m.sel_accessor})"
            arg_suffix = f", {args}" if args else ""
            lines += [
                f"_{p}_INLINE {ret} {ns}::{cpp_name}::{m.cpp_name}({params_str})",
                "{",
                f"    return Object::sendMessage<{ret}>("
                f"_{p}_PRIVATE_CLS({cls.name}), {sel}{arg_suffix});",
                "}",
                "",
            ]

        # Instance properties
        for prop in instance_props:
            cpp_type = self._resolve(prop.objc_type, cls.name)
            sel = f"_{p}_PRIVATE_SEL({prop.name})"
            lines += [
                f"_{p}_INLINE {cpp_type} {ns}::{cpp_name}::{prop.name}() const",
                "{",
                f"    return Object::sendMessage<{cpp_type}>(this, {sel});",
                "}",
                "",
            ]
            if not prop.is_readonly:
                sname = self._setter_name(prop.name)
                ssel = f"_{p}_PRIVATE_SEL({sname}_)"
                lines += [
                    f"_{p}_INLINE void {ns}::{cpp_name}::{sname}"
                    f"({cpp_type} {prop.name})",
                    "{",
                    f"    Object::sendMessage<void>(this, {ssel}, {prop.name});",
                    "}",
                    "",
                ]

        # Instance methods
        for m in instance_methods:
            ret = self._resolve(m.return_type, cls.name)
            params_str = self._fmt_params(m.params, cls.name)
            args = self._fmt_args(m.params)
            sel = f"_{p}_PRIVATE_SEL({m.sel_accessor})"
            arg_suffix = f", {args}" if args else ""
            lines += [
                f"_{p}_INLINE {ret} {ns}::{cpp_name}::{m.cpp_name}"
                f"({params_str}) const",
                "{",
                f"    return Object::sendMessage<{ret}>(this, {sel}{arg_suffix});",
                "}",
                "",
            ]

        return "\n".join(lines)

    def generate_umbrella(self, fw_name: str) -> str:
        p = self.prefix
        lines = [
            "#pragma once",
            "",
            f'#include "{p}Defines.hpp"',
        ]
        for header in sorted(self.generated_headers):
            lines.append(f'#include "{header}"')
        lines.append("")
        return "\n".join(lines)

    # ── Formatting helpers ────────────────────────────────────────────

    def _fmt_params(self, params: list[ObjCParam], cls_name: str) -> str:
        if not params:
            return ""
        parts = []
        for p in params:
            cpp_type = self._resolve(p.objc_type, cls_name)
            parts.append(f"{cpp_type} {p.name}")
        return ", ".join(parts)

    def _fmt_args(self, params: list[ObjCParam]) -> str:
        return ", ".join(p.name for p in params)
