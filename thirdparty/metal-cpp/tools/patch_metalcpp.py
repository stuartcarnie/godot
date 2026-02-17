#!/usr/bin/env python3
"""Patch metal-cpp with Godot-specific amendments.

Reads a declarative YAML config, parses SDK headers via libclang to get
type information, generates metal-cpp C++ code, and applies amendments
to the upstream files.

Usage:
    python patch_metalcpp.py <metal-cpp-dir> --config patches.yaml
    python patch_metalcpp.py <metal-cpp-dir> --zip metal-cpp_26.zip
"""

from __future__ import annotations

import argparse
import logging
import re
import shutil
import subprocess
import sys
import tempfile
import zipfile
from dataclasses import dataclass
from pathlib import Path

import yaml

log = logging.getLogger(__name__)

TOOL_DIR = Path(__file__).resolve().parent

sys.path.insert(0, str(TOOL_DIR))
from clang.cindex import CursorKind
from metalcpp_common import (
    CodeGenerator,
    ObjCClass,
    ObjCMethod,
    ObjCParser,
    ObjCProperty,
    TypeResolver,
    resolve_type,
    selector_accessor,
    setter_name,
    strip_objc_prefix,
)


# Typed string constant: typedef NSString * SomeType NS_TYPED_ENUM
# with associated extern const SomeType SomeTypeValue declarations.
@dataclass
class TypedStringConst:
    objc_typedef: str  # e.g. "CADynamicRange"
    constants: list[str]  # e.g. ["CADynamicRangeAutomatic", ...]


# ── Config loading ───────────────────────────────────────────────────────


def load_config(config_path: Path) -> dict:
    """Load and validate patches.yaml."""
    with open(config_path) as f:
        cfg = yaml.safe_load(f)
    if "frameworks" not in cfg:
        log.error("Config missing 'frameworks' key")
        sys.exit(1)
    return cfg


def get_sdk_path(xcrun_sdk: str) -> Path:
    result = subprocess.run(
        ["xcrun", "--sdk", xcrun_sdk, "--show-sdk-path"],
        capture_output=True,
        text=True,
        check=True,
    )
    return Path(result.stdout.strip())


def extract_zip(zip_path: Path, dest_dir: Path) -> None:
    """Extract upstream metal-cpp zip into dest_dir, preserving the tools/ dir."""
    if not zip_path.exists():
        log.error("Zip file not found: %s", zip_path)
        sys.exit(1)

    with zipfile.ZipFile(zip_path) as zf:
        # Find the top-level directory inside the zip (e.g. "metal-cpp/").
        top_dirs = {name.split("/")[0] for name in zf.namelist() if "/" in name}
        if len(top_dirs) != 1:
            log.error("Expected single top-level dir in zip, got: %s", top_dirs)
            sys.exit(1)
        zip_root = top_dirs.pop()

        # Collect the set of top-level names present in the zip so we only
        # replace what the zip ships (preserving tools/, patches/, etc.).
        zip_top_names: set[str] = set()
        for name in zf.namelist():
            rel = name[len(zip_root) + 1 :]
            if rel:
                zip_top_names.add(rel.split("/")[0])

        for name in zip_top_names:
            child = dest_dir / name
            if child.is_dir():
                shutil.rmtree(child)
            elif child.exists():
                child.unlink()

        # Extract, stripping the zip's top-level directory.
        for info in zf.infolist():
            if info.is_dir():
                continue
            # Strip "metal-cpp/" prefix to get relative path.
            rel = info.filename[len(zip_root) + 1 :]
            if not rel:
                continue
            out_path = dest_dir / rel
            out_path.parent.mkdir(parents=True, exist_ok=True)
            with zf.open(info) as src, open(out_path, "wb") as dst:
                dst.write(src.read())

    log.info("Extracted %s -> %s", zip_path.name, dest_dir)


# ── File patching utilities ──────────────────────────────────────────────


def read_file(path: Path) -> str:
    return path.read_text()


def write_file(path: Path, content: str) -> None:
    if path.exists() and path.read_text() == content:
        return
    path.write_text(content)


def insert_before_line(text: str, marker: str, insertion: str) -> str:
    """Insert text before the first line containing marker."""
    lines = text.split("\n")
    for i, line in enumerate(lines):
        if marker in line:
            insert_lines = insertion.split("\n")
            lines[i:i] = insert_lines
            return "\n".join(lines)
    raise ValueError(f"Marker not found: {marker!r}")


def append_after_last(text: str, pattern: str, insertion: str) -> str:
    """Append insertion after the last occurrence of a regex pattern."""
    matches = list(re.finditer(pattern, text))
    if not matches:
        raise ValueError(f"Pattern not found: {pattern!r}")
    last = matches[-1]
    pos = last.end()
    return text[:pos] + "\n" + insertion + text[pos:]


# ── Forward declarations ─────────────────────────────────────────────────


def apply_forward_declarations(
    metal_cpp_dir: Path,
    fw_name: str,
    namespace: str,
    names: list[str],
) -> None:
    """Inject forward declarations into *Defines.hpp."""
    defines_path = metal_cpp_dir / fw_name / f"{namespace}Defines.hpp"

    content = read_file(defines_path)

    # Check if already applied.
    if f"namespace {namespace} {{" in content and all(f"class {n};" in content for n in names):
        log.info("    forward_declarations: already applied")
        return

    # Build the block.
    decls = "\n".join(f"class {n};" for n in names)
    block = (
        f"\n// Forward declarations to avoid conflicts with Godot types"
        f" (String, Object, Error)\n"
        f"namespace {namespace} {{\n{decls}\n}} // namespace {namespace}\n"
    )

    # Insert after the first //--- separator following #pragma once.
    pattern = r"(#pragma once\n+//[-]+\n)"
    m = re.search(pattern, content)
    if not m:
        log.error("    Cannot find insertion point in %s", defines_path)
        return

    pos = m.end()
    content = content[:pos] + block + content[pos:]
    write_file(defines_path, content)
    log.info("    forward_declarations: applied to %s", defines_path.name)


# ── Move to public ──────────────────────────────────────────────────────


def apply_move_to_public(
    metal_cpp_dir: Path,
    fw_name: str,
    strip_prefix: str,
    objc_class_name: str,
    method_names: list[str],
) -> None:
    """Move method declarations from protected/private to public in a header."""
    header_path = metal_cpp_dir / fw_name / f"{objc_class_name}.hpp"
    content = read_file(header_path)

    for name in method_names:
        # Match template line + method declaration line as a pair.
        pattern = re.compile(
            rf"^([ \t]+template\s*<[^>]*>\n)"
            rf"([ \t]+static\s+\S+\s+{re.escape(name)}\(.*;\n)",
            re.MULTILINE,
        )
        m = pattern.search(content)
        if not m:
            log.error("    move_to_public: %s not found in %s", name, header_path.name)
            continue

        decl = m.group(0)

        # Check if already in public section.
        public_pos = content.rfind("public:", 0, m.start())
        protected_pos = content.rfind("protected:", 0, m.start())
        if public_pos > protected_pos:
            log.info("    move_to_public: %s already public", name)
            continue

        # Remove from current location.
        content = content[: m.start()] + content[m.end() :]

        # Insert just before "protected:" (end of public section).
        protected_match = re.search(r"^(protected:)", content, re.MULTILINE)
        if not protected_match:
            log.error("    move_to_public: no protected: section in %s", header_path.name)
            continue

        insert_pos = protected_match.start()
        content = content[:insert_pos] + decl + "\n" + content[insert_pos:]
        log.info("    move_to_public: %s moved to public in %s", name, header_path.name)

    write_file(header_path, content)


# ── Base class amendment ─────────────────────────────────────────────────


def apply_base_class(
    metal_cpp_dir: Path,
    fw_name: str,
    prefix: str,
    strip_prefix: str,
    objc_name: str,
    new_base: str,
) -> None:
    """Change NS::Referencing<CppName> to NS::Referencing<CppName, NewBase>."""
    header_path = metal_cpp_dir / fw_name / f"{objc_name}.hpp"
    if not header_path.exists():
        log.error("    base_class: %s not found", header_path)
        return

    content = read_file(header_path)
    cpp_name = strip_objc_prefix(objc_name, strip_prefix)

    old = f"NS::Referencing<{cpp_name}>"
    new = f"NS::Referencing<{cpp_name}, {new_base}>"

    if new in content:
        log.info("    base_class %s: already applied", objc_name)
        return
    if old not in content:
        log.error("    base_class %s: pattern %r not found", objc_name, old)
        return

    # Add #include for the base class header if not present.
    base_header = f"{prefix}{new_base}.hpp"
    include_directive = f'#include "{base_header}"'
    if include_directive not in content:
        # Insert after the last existing #include in the header.
        last_include = -1
        for m in re.finditer(r"^#include\s+.+$", content, re.MULTILINE):
            last_include = m.end()
        if last_include != -1:
            content = content[:last_include] + "\n" + include_directive + content[last_include:]
            log.info("    base_class %s: added %s", objc_name, include_directive)

    content = content.replace(old, new, 1)
    write_file(header_path, content)
    log.info("    base_class %s: %s -> %s", objc_name, old, new)


# ── LoadSymbol fallback for older SDKs ────────────────────────────────────


def apply_loadsymbol_fallback(
    metal_cpp_dir: Path,
    fw_name: str,
    namespace: str,
    prefix: str,
) -> None:
    """Add LoadSymbol/dlsym fallback for _PRIVATE_DEF_STR on older SDKs.

    When building against SDKs older than 26.0, typed string constants
    (e.g. CADynamicRange*) are not present in the SDK's tbd files, so
    weak_import fails at link time.  This mirrors the pattern used by
    MTLPrivate.hpp: use weak_import when the SDK provides the symbols,
    fall back to dlsym otherwise.
    """
    private_path = metal_cpp_dir / fw_name / f"{prefix}Private.hpp"
    content = read_file(private_path)

    # Idempotency check.
    if f"{namespace}::Private::LoadSymbol" in content:
        log.info("    loadsymbol_fallback: already applied")
        return

    tag = f"_{prefix}_PRIVATE_DEF_STR"

    # Locate the implementation guard.
    impl_guard = f"{prefix}_PRIVATE_IMPLEMENTATION"
    impl_pos = content.find(impl_guard)
    if impl_pos == -1:
        log.error("    loadsymbol_fallback: %s not found in %s", impl_guard, private_path.name)
        return

    # Find the implementation-section macro (multi-line #define with continuations).
    # Search from impl_pos — the first occurrence is always the impl version.
    macro_start = content.find(f"#define {tag}(type, symbol)", impl_pos)
    if macro_start == -1:
        log.error("    loadsymbol_fallback: %s macro not found in %s", tag, private_path.name)
        return

    # Walk continuation lines (ending with \) to find the full macro span.
    pos = macro_start
    while True:
        eol = content.find("\n", pos)
        if eol == -1:
            eol = len(content)
            break
        if not content[pos:eol].rstrip().endswith("\\"):
            break
        pos = eol + 1
    old_macro = content[macro_start:eol]
    abs_start = macro_start
    abs_end = eol

    const_tag = f"_{prefix}_PRIVATE_DEF_CONST"

    # Build the weak_import macros (SDK >= 26).
    weak_const_macro = (
        f"#define {const_tag}(type, symbol)              \\\n"
        f"    _{prefix}_EXTERN type const {prefix}##symbol _{prefix}_PRIVATE_IMPORT; \\\n"
        f"    type const                         {namespace}::symbol ="
        f" (nullptr != &{prefix}##symbol) ? {prefix}##symbol : nullptr"
    )

    # Build the dlsym fallback macros (SDK < 26).
    dlsym_str = (
        f"#define {tag}(type, symbol)    \\\n"
        f"    _{prefix}_EXTERN type const {prefix}##symbol;    \\\n"
        f"    type const             {namespace}::symbol ="
        f" {namespace}::Private::LoadSymbol<type>(\"{prefix}\" #symbol)"
    )
    dlsym_const = (
        f"#define {const_tag}(type, symbol)    \\\n"
        f"    _{prefix}_EXTERN type const {prefix}##symbol;    \\\n"
        f"    type const             {namespace}::symbol ="
        f" {namespace}::Private::LoadSymbol<type>(\"{prefix}\" #symbol)"
    )

    # Build the extern-only macros (non-implementation).
    extern_const = f"#define {const_tag}(type, symbol) extern type const {namespace}::symbol"

    replacement = (
        f"\n#include <dlfcn.h>\n"
        f"\n"
        f"namespace {namespace}::Private\n"
        f"{{\n"
        f"    template <typename _Type>\n"
        f"    inline _Type const LoadSymbol(const char* pSymbol)\n"
        f"    {{\n"
        f"        const _Type* pAddress = static_cast<_Type*>(dlsym(RTLD_DEFAULT, pSymbol));\n"
        f"\n"
        f"        return pAddress ? *pAddress : nullptr;\n"
        f"    }}\n"
        f"}} // {namespace}::Private\n"
        f"\n"
        f"#if defined(__MAC_26_0) || defined(__IPHONE_26_0) || defined(__TVOS_26_0)\n"
        f"\n"
        f"{old_macro}\n"
        f"\n"
        f"{weak_const_macro}\n"
        f"\n"
        f"#else\n"
        f"\n"
        f"{dlsym_str}\n"
        f"\n"
        f"{dlsym_const}\n"
        f"\n"
        f"#endif"
    )

    content = content[:abs_start] + replacement + content[abs_end:]

    # Also add _PRIVATE_DEF_CONST to the extern-only #else section.
    extern_str_marker = f"#define {tag}(type, symbol) extern type const {namespace}::symbol"
    marker_pos = content.find(extern_str_marker)
    if marker_pos != -1:
        insert_pos = marker_pos + len(extern_str_marker)
        content = content[:insert_pos] + "\n" + extern_const + content[insert_pos:]

    write_file(private_path, content)
    log.info("    loadsymbol_fallback: applied to %s", private_path.name)


# ── Parse SDK for a class ────────────────────────────────────────────────


def _sdk_header_path(sdk_path: Path, fw_name: str, header_name: str) -> Path:
    return sdk_path / "System" / "Library" / "Frameworks" / f"{fw_name}.framework" / "Headers" / header_name


def parse_objc_class(
    sdk_path: Path,
    fw_name: str,
    objc_class_name: str,
) -> ObjCClass | None:
    """Parse an ObjC header from the SDK and return the named class."""
    header_path = _sdk_header_path(sdk_path, fw_name, f"{objc_class_name}.h")
    if not header_path.exists():
        log.error("    SDK header not found: %s", header_path)
        return None

    parser = ObjCParser(sdk_path)
    data = parser.parse_header(header_path)
    for cls in data.classes:
        if cls.name == objc_class_name:
            return cls
    log.error("    Class %s not found in %s", objc_class_name, header_path)
    return None


def parse_typed_string_constants(
    sdk_path: Path,
    fw_name: str,
    header_name: str,
) -> list[TypedStringConst]:
    """Find typedef NSString * ... NS_TYPED_ENUM and their extern constants."""
    header_path = _sdk_header_path(sdk_path, fw_name, header_name)
    if not header_path.exists():
        return []

    index = __import__("clang.cindex", fromlist=["Index"]).Index.create()
    tu = index.parse(
        str(header_path),
        args=["-x", "objective-c", "-isysroot", str(sdk_path), "-fno-objc-arc", "-Wno-everything"],
        options=0,
    )

    target = str(header_path)
    # Collect typedefs that alias NSString *.
    string_typedefs: set[str] = set()
    # Collect extern constants grouped by typedef.
    constants_by_typedef: dict[str, list[str]] = {}

    for c in tu.cursor.get_children():
        if not c.location.file or c.location.file.name != target:
            continue
        if c.kind == CursorKind.TYPEDEF_DECL:
            if c.underlying_typedef_type.spelling == "NSString *":
                string_typedefs.add(c.spelling)
                constants_by_typedef[c.spelling] = []
        elif c.kind == CursorKind.VAR_DECL:
            # type.spelling is e.g. "const CADynamicRange"
            base_type = c.type.spelling.replace("const ", "").strip()
            if base_type in string_typedefs:
                constants_by_typedef[base_type].append(c.spelling)

    return [
        TypedStringConst(objc_typedef=td, constants=consts)
        for td, consts in constants_by_typedef.items()
        if consts  # only include typedefs that have constants
    ]


# ── Code snippet generation ──────────────────────────────────────────────


def _cpp_method_name(method: ObjCMethod) -> str:
    """C++ method name following metal-cpp convention.

    init-family selectors (initWith*, initFor*, etc.) are simplified to
    ``init`` so that C++ overloading distinguishes the variants, matching
    how upstream metal-cpp names these methods.
    """
    name = method.cpp_name
    if name.startswith("init") and len(name) > 4 and name[4].isupper():
        return "init"
    return name


def gen_property_declaration(
    resolver: TypeResolver,
    prop: ObjCProperty,
    namespace: str,
    cpp_class_name: str,
) -> list[str]:
    """Generate class body declaration lines for a property."""
    cpp_type = resolve_type(resolver, prop.objc_type, namespace, cpp_class_name)
    lines = [f"    {cpp_type} {prop.name}() const;"]
    if not prop.is_readonly:
        lines.append(f"    void {setter_name(prop.name)}({cpp_type} {prop.name});")
    return lines


def gen_property_impl(
    resolver: TypeResolver,
    prop: ObjCProperty,
    namespace: str,
    prefix: str,
    cpp_class_name: str,
) -> str:
    """Generate inline implementation for a property."""
    cpp_type = resolve_type(resolver, prop.objc_type, namespace, cpp_class_name)
    parts = []

    # Getter
    parts.append(
        f"_{prefix}_INLINE {cpp_type} {namespace}::{cpp_class_name}::{prop.name}() const\n"
        f"{{\n"
        f"    return Object::sendMessage<{cpp_type}>"
        f"(this, _{prefix}_PRIVATE_SEL({prop.name}));\n"
        f"}}"
    )

    # Setter
    if not prop.is_readonly:
        sname = setter_name(prop.name)
        sacc = f"{sname}_"
        parts.append(
            f"_{prefix}_INLINE void {namespace}::{cpp_class_name}::{sname}"
            f"({cpp_type} {prop.name})\n"
            f"{{\n"
            f"    Object::sendMessage<void>"
            f"(this, _{prefix}_PRIVATE_SEL({sacc}), {prop.name});\n"
            f"}}"
        )

    return "\n\n".join(parts)


def gen_method_declaration(
    resolver: TypeResolver,
    method: ObjCMethod,
    namespace: str,
    cpp_class_name: str,
) -> list[str]:
    """Generate class body declaration lines for a method."""
    ret = resolve_type(resolver, method.return_type, namespace, cpp_class_name)
    params = ", ".join(
        f"{resolve_type(resolver, p.objc_type, namespace, cpp_class_name)} {p.name}" for p in method.params
    )
    static = "static " if method.is_class_method else ""
    const = "" if method.is_class_method else " const"
    # instancetype for init methods returns the class pointer.
    if ret == f"{namespace}::{cpp_class_name}*" and not method.is_class_method:
        # init-style method, no const
        const = ""
    name = _cpp_method_name(method)
    return [f"    {static}{ret} {name}({params}){const};"]


def gen_method_impl(
    resolver: TypeResolver,
    method: ObjCMethod,
    namespace: str,
    prefix: str,
    cpp_class_name: str,
    objc_class_name: str,
) -> str:
    """Generate inline implementation for a method."""
    ret = resolve_type(resolver, method.return_type, namespace, cpp_class_name)
    params = ", ".join(
        f"{resolve_type(resolver, p.objc_type, namespace, cpp_class_name)} {p.name}" for p in method.params
    )
    args = ", ".join(p.name for p in method.params)
    sel_acc = selector_accessor(method.selector)
    name = _cpp_method_name(method)

    if method.is_class_method:
        receiver = f"_{prefix}_PRIVATE_CLS({objc_class_name})"
        const = ""
    else:
        receiver = "this"
        const = " const"
        # init-style: no const
        if ret == f"{namespace}::{cpp_class_name}*" and name.startswith("init"):
            const = ""

    arg_suffix = f", {args}" if args else ""

    return (
        f"_{prefix}_INLINE {ret} {namespace}::{cpp_class_name}::{name}"
        f"({params}){const}\n"
        f"{{\n"
        f"    return Object::sendMessage<{ret}>"
        f"({receiver}, _{prefix}_PRIVATE_SEL({sel_acc}){arg_suffix});\n"
        f"}}"
    )


def gen_selector_entries(
    prop: ObjCProperty | None = None,
    method: ObjCMethod | None = None,
) -> list[tuple[str, str]]:
    """Return (accessor, objc_selector) pairs for Private.hpp."""
    entries = []
    if prop:
        entries.append((prop.name, prop.name))
        if not prop.is_readonly:
            sname = setter_name(prop.name)
            entries.append((f"{sname}_", f"{sname}:"))
    if method:
        entries.append((selector_accessor(method.selector), method.selector))
    return entries


# ── File insertion operations ────────────────────────────────────────────


def _find_class_range(
    content: str,
    cpp_class_name: str,
) -> tuple[int, int] | None:
    """Find the opening '{' and closing '}' positions of a class declaration."""
    class_match = re.search(rf"class\s+{re.escape(cpp_class_name)}\s*:", content)
    if not class_match:
        return None
    brace_start = content.index("{", class_match.start())
    depth = 1
    i = brace_start + 1
    while i < len(content) and depth > 0:
        if content[i] == "{":
            depth += 1
        elif content[i] == "}":
            depth -= 1
        i += 1
    return brace_start, i - 1


def class_has_member(header_path: Path, cpp_class_name: str, member_name: str) -> bool:
    """Check if a class already has a member with the given name."""
    content = read_file(header_path)
    rng = _find_class_range(content, cpp_class_name)
    if rng is None:
        return False
    return f"{member_name}(" in content[rng[0] : rng[1]]


def insert_in_class_body(
    header_path: Path,
    cpp_class_name: str,
    new_lines: list[str],
) -> None:
    """Insert declaration lines before the closing }; of a class."""
    content = read_file(header_path)
    rng = _find_class_range(content, cpp_class_name)
    if rng is None:
        log.error("    Class %s not found in %s", cpp_class_name, header_path)
        return

    brace_start, close_brace = rng

    # Check idempotency: see if the first new line is already present.
    first_line = new_lines[0].strip()
    if first_line in content[brace_start:close_brace]:
        log.info("    %s: declarations already present", cpp_class_name)
        return

    insertion = "\n".join(new_lines) + "\n"
    content = content[:close_brace] + insertion + content[close_brace:]
    write_file(header_path, content)


def append_inline_impl(header_path: Path, impl_code: str) -> None:
    """Append inline implementation before the final //--- separator."""
    content = read_file(header_path)

    # Check idempotency: extract the function name from the impl.
    first_line = impl_code.split("\n")[0]
    if first_line in content:
        log.info("    Inline impl already present in %s", header_path.name)
        return

    # Find last //--- separator line.
    separator = "//---"
    last_sep = content.rfind(separator)
    if last_sep == -1:
        # No separator, just append.
        content += "\n" + impl_code + "\n"
    else:
        # Find start of the last separator line.
        line_start = content.rfind("\n", 0, last_sep)
        if line_start == -1:
            line_start = 0
        else:
            line_start += 1
        content = content[:line_start] + impl_code + "\n\n" + content[line_start:]

    write_file(header_path, content)


def get_existing_selectors(private_path: Path, prefix: str) -> dict[str, str]:
    """Return a mapping of ObjC selector string → accessor name from Private.hpp."""
    content = read_file(private_path)
    result: dict[str, str] = {}
    # Match: _PREFIX_PRIVATE_DEF_SEL(accessor,\n            "selector");
    pattern = re.compile(
        rf'_{prefix}_PRIVATE_DEF_SEL\((\w+),\s*"([^"]+)"\)',
    )
    for m in pattern.finditer(content):
        result[m.group(2)] = m.group(1)
    return result


def insert_selector_in_private(
    private_path: Path,
    prefix: str,
    accessor: str,
    selector_str: str,
) -> None:
    """Insert a selector registration into *Private.hpp in sorted position."""
    content = read_file(private_path)

    # Check idempotency.
    check = f"_PRIVATE_DEF_SEL({accessor},"
    if check in content:
        log.debug("    Selector %s already in %s", accessor, private_path.name)
        return

    entry = f'        _{prefix}_PRIVATE_DEF_SEL({accessor},\n            "{selector_str}");'

    # Find the Selector namespace block.
    sel_ns = content.find("namespace Selector")
    if sel_ns == -1:
        log.error("    namespace Selector not found in %s", private_path.name)
        return

    # Find all existing _PRIVATE_DEF_SEL entries and their accessors.
    sel_pattern = re.compile(rf"_{prefix}_PRIVATE_DEF_SEL\((\w+),")
    existing = []
    for m in sel_pattern.finditer(content, sel_ns):
        existing.append((m.group(1), m.start()))

    # Find where to insert (sorted by accessor name).
    insert_pos = None
    insert_before = False
    for acc, pos in existing:
        if acc > accessor:
            # Insert before this entry — find start of its line.
            line_start = content.rfind("\n", 0, pos)
            insert_pos = line_start + 1 if line_start != -1 else pos
            insert_before = True
            break

    if insert_pos is None:
        # Insert after the last entry.
        if existing:
            last_acc, last_pos = existing[-1]
            semi = content.index(";", last_pos)
            insert_pos = semi + 1
        else:
            # Empty selector block — insert after opening {.
            brace_start = content.index("{", sel_ns)
            insert_pos = brace_start + 1

    if insert_before:
        # insert_pos is right after a \n, so no leading newline needed.
        content = content[:insert_pos] + entry + "\n" + content[insert_pos:]
    else:
        # insert_pos is right after a ; or {, need leading newline.
        content = content[:insert_pos] + "\n" + entry + "\n" + content[insert_pos:]
    write_file(private_path, content)
    log.info("    Selector: added %s to %s", accessor, private_path.name)


def insert_class_in_private(
    private_path: Path,
    prefix: str,
    objc_class_name: str,
) -> None:
    """Insert a class registration into *Private.hpp."""
    content = read_file(private_path)

    check = f"_PRIVATE_DEF_CLS({objc_class_name})"
    if check in content:
        log.debug("    Class %s already in %s", objc_class_name, private_path.name)
        return

    entry = f"        _{prefix}_PRIVATE_DEF_CLS({objc_class_name});"

    # Find the Class namespace block.
    cls_ns = content.find("namespace Class")
    if cls_ns == -1:
        log.error("    namespace Class not found in %s", private_path.name)
        return

    # Insert before the closing } // Class.
    close = content.find("} // Class", cls_ns)
    if close == -1:
        log.error("    } // Class not found in %s", private_path.name)
        return

    content = content[:close] + entry + "\n    " + content[close:]
    write_file(private_path, content)
    log.info("    Class: added %s to %s", objc_class_name, private_path.name)


def insert_include_in_umbrella(
    umbrella_path: Path,
    include_name: str,
) -> None:
    """Add #include to umbrella header in sorted position."""
    content = read_file(umbrella_path)

    directive = f'#include "{include_name}"'
    if directive in content:
        log.debug("    Include %s already in %s", include_name, umbrella_path.name)
        return

    # Collect existing non-Defines #include lines with their indices.
    lines = content.split("\n")
    includes: list[tuple[int, str]] = []
    for i, line in enumerate(lines):
        if line.startswith('#include "') and "Defines" not in line:
            includes.append((i, line))

    if not includes:
        # No includes found, insert before last //--- separator.
        for i in range(len(lines) - 1, -1, -1):
            if lines[i].startswith("//---"):
                lines.insert(i, directive)
                break
    else:
        # Insert in sorted position among existing includes.
        insert_idx = None
        for idx, line in includes:
            if directive < line:
                insert_idx = idx
                break
        if insert_idx is None:
            # Goes after the last include.
            insert_idx = includes[-1][0] + 1
        lines.insert(insert_idx, directive)

    write_file(umbrella_path, "\n".join(lines))
    log.info("    Umbrella: added %s", include_name)


# ── Amendment processors ─────────────────────────────────────────────────


def _prepare_amend(
    metal_cpp_dir: Path,
    sdk_path: Path,
    fw_name: str,
    prefix: str,
    strip_prefix: str,
    objc_class_name: str,
) -> tuple[ObjCClass, str, Path, Path] | None:
    """Shared setup for amend_properties / amend_methods."""
    cls = parse_objc_class(sdk_path, fw_name, objc_class_name)
    if not cls:
        return None
    cpp_class_name = strip_objc_prefix(objc_class_name, strip_prefix)
    header_path = metal_cpp_dir / fw_name / f"{objc_class_name}.hpp"
    private_path = metal_cpp_dir / fw_name / f"{prefix}Private.hpp"
    return cls, cpp_class_name, header_path, private_path


def process_amend_properties(
    metal_cpp_dir: Path,
    sdk_path: Path,
    fw_name: str,
    namespace: str,
    prefix: str,
    strip_prefix: str,
    objc_class_name: str,
    prop_names: list[str],
    resolver: TypeResolver,
) -> None:
    """Add properties to an existing class."""
    prepared = _prepare_amend(
        metal_cpp_dir,
        sdk_path,
        fw_name,
        prefix,
        strip_prefix,
        objc_class_name,
    )
    if not prepared:
        return
    cls, cpp_class_name, header_path, private_path = prepared

    for prop_name in prop_names:
        prop = next((p for p in cls.properties if p.name == prop_name), None)
        if not prop:
            log.error("    Property %s not found on %s", prop_name, objc_class_name)
            continue

        # Skip if already present (possibly with different type).
        if class_has_member(header_path, cpp_class_name, prop_name):
            log.info("    add_property: %s.%s already exists", objc_class_name, prop_name)
        else:
            log.info("    add_property: %s.%s", objc_class_name, prop_name)

            # 1. Class declaration.
            decl_lines = gen_property_declaration(resolver, prop, namespace, cpp_class_name)
            insert_in_class_body(header_path, cpp_class_name, decl_lines)

            # 2. Inline implementation.
            impl = gen_property_impl(resolver, prop, namespace, prefix, cpp_class_name)
            append_inline_impl(header_path, impl)

        # 3. Selector registration (always ensure present).
        for acc, sel_str in gen_selector_entries(prop=prop):
            insert_selector_in_private(private_path, prefix, acc, sel_str)


def process_amend_methods(
    metal_cpp_dir: Path,
    sdk_path: Path,
    fw_name: str,
    namespace: str,
    prefix: str,
    strip_prefix: str,
    objc_class_name: str,
    selectors: list[str],
    resolver: TypeResolver,
) -> None:
    """Add methods to an existing class."""
    prepared = _prepare_amend(
        metal_cpp_dir,
        sdk_path,
        fw_name,
        prefix,
        strip_prefix,
        objc_class_name,
    )
    if not prepared:
        return
    cls, cpp_class_name, header_path, private_path = prepared

    # Read Private.hpp once to check existing selectors.
    private_content = read_file(private_path)

    for selector in selectors:
        method = next((m for m in cls.methods if m.selector == selector), None)
        if not method:
            log.error("    Method %s not found on %s", selector, objc_class_name)
            continue

        # Check if the selector is already registered (method already wrapped).
        sel_acc = selector_accessor(selector)
        if f"_PRIVATE_DEF_SEL({sel_acc}," in private_content:
            log.info("    add_method: %s.%s already wrapped", objc_class_name, selector)
            continue

        log.info("    add_method: %s.%s", objc_class_name, selector)

        # 1. Class declaration.
        decl_lines = gen_method_declaration(resolver, method, namespace, cpp_class_name)
        insert_in_class_body(header_path, cpp_class_name, decl_lines)

        # 2. Inline implementation.
        impl = gen_method_impl(
            resolver,
            method,
            namespace,
            prefix,
            cpp_class_name,
            objc_class_name,
        )
        append_inline_impl(header_path, impl)

        # 3. Selector registration.
        for acc, sel_str in gen_selector_entries(method=method):
            insert_selector_in_private(private_path, prefix, acc, sel_str)


# ── Add new types ────────────────────────────────────────────────────────


def process_add_type(
    metal_cpp_dir: Path,
    sdk_path: Path,
    fw_name: str,
    namespace: str,
    prefix: str,
    strip_prefix: str,
    objc_class_name: str,
    type_config: dict,
    resolver: TypeResolver,
) -> bool:
    """Generate a complete .hpp for a new type.  Returns True if typed constants were emitted."""
    cls = parse_objc_class(sdk_path, fw_name, objc_class_name)
    if not cls:
        return False

    # Filter to requested properties/methods.
    req_props = type_config.get("properties", [])
    req_methods = type_config.get("methods", [])

    if req_props != ["*"]:
        cls.properties = [p for p in cls.properties if p.name in req_props]
    if req_methods != ["*"]:
        cls.methods = [m for m in cls.methods if m.selector in req_methods]

    log.info("    add_type: %s (%d props, %d methods)", objc_class_name, len(cls.properties), len(cls.methods))

    # Discover typed string constants used by properties and register them
    # in the TypeResolver so generate_class_header resolves the types.
    all_typed_consts = parse_typed_string_constants(
        sdk_path,
        fw_name,
        f"{objc_class_name}.h",
    )
    prop_types = {p.objc_type for p in cls.properties}
    used_typed_consts = [tc for tc in all_typed_consts if tc.objc_typedef in prop_types]
    for tc in used_typed_consts:
        cpp_type_name = strip_objc_prefix(tc.objc_typedef, strip_prefix)
        resolver.register(tc.objc_typedef, f"{namespace}::{cpp_type_name}")

    # Use CodeGenerator to produce the complete header.
    gen = CodeGenerator(
        namespace=namespace,
        prefix=prefix,
        strip_prefix=strip_prefix,
        resolver=resolver,
        rel_to_root="../",
    )
    gen.collect_selectors(cls)
    header_content = gen.generate_class_header(cls)

    # Remap accessors that already exist in Private.hpp under a different name.
    private_path = metal_cpp_dir / fw_name / f"{prefix}Private.hpp"
    existing_sels = get_existing_selectors(private_path, prefix)
    for accessor, sel_str in list(gen.all_selectors.items()):
        if sel_str in existing_sels and existing_sels[sel_str] != accessor:
            old_acc = accessor
            new_acc = existing_sels[sel_str]
            header_content = header_content.replace(f"_PRIVATE_SEL({old_acc})", f"_PRIVATE_SEL({new_acc})")
            del gen.all_selectors[old_acc]
            log.info("    Remapped accessor %s -> %s", old_acc, new_acc)

    # Inject auto-detected typed string constants.
    if used_typed_consts:
        header_content = _inject_typed_constants(
            header_content,
            namespace,
            prefix,
            strip_prefix,
            used_typed_consts,
        )

    # Write the header file (always regenerate — it's fully generated).
    header_path = metal_cpp_dir / fw_name / f"{objc_class_name}.hpp"
    write_file(header_path, header_content)
    log.info("    -> %s", header_path.name)

    # Update umbrella header.
    umbrella_path = metal_cpp_dir / fw_name / f"{fw_name}.hpp"
    insert_include_in_umbrella(umbrella_path, f"{objc_class_name}.hpp")

    # Update Private.hpp: class registration.
    insert_class_in_private(private_path, prefix, objc_class_name)

    # Update Private.hpp: selector registrations (only new ones).
    for accessor, sel_str in sorted(gen.all_selectors.items()):
        insert_selector_in_private(private_path, prefix, accessor, sel_str)

    # Note: typed constant _PRIVATE_DEF_STR entries go in the class header
    # (not Private.hpp), since the type alias must be visible. This is handled
    # by _inject_typed_constants.

    return bool(used_typed_consts)


def _inject_typed_constants(
    header_content: str,
    namespace: str,
    prefix: str,
    strip_prefix: str,
    typed_consts: list[TypedStringConst],
) -> str:
    """Inject type aliases, constant declarations, and definitions into a generated header."""
    alias_lines = []
    const_lines = []
    def_lines = []
    for tc in typed_consts:
        cpp_type_name = strip_objc_prefix(tc.objc_typedef, strip_prefix)

        alias_lines.append(f"using {cpp_type_name} = NS::String*;")
        qualified = f"{namespace}::{cpp_type_name}"
        for objc_const in tc.constants:
            cpp_const = strip_objc_prefix(objc_const, strip_prefix)
            const_lines.append(f"_{prefix}_CONST({cpp_type_name}, {cpp_const});")
            def_lines.append(f"_{prefix}_PRIVATE_DEF_CONST({qualified}, {cpp_const});")

        log.info("    typed_constants: %s (%d constants)", tc.objc_typedef, len(tc.constants))

    # Insert type alias + _CONST declarations into the namespace block,
    # before the class definition.
    ns_pattern = rf"(namespace {re.escape(namespace)}\n\{{\n)"
    m = re.search(ns_pattern, header_content)
    if not m:
        log.error("    Cannot find namespace block for typed constants")
        return header_content

    insertion = "\n".join(alias_lines + [""] + const_lines + [""])
    pos = m.end()
    header_content = header_content[:pos] + insertion + "\n" + header_content[pos:]

    # Append _PRIVATE_DEF_CONST definitions at the end of the file
    # (after namespace close and inline impls).
    header_content = header_content.rstrip("\n") + "\n\n" + "\n".join(def_lines) + "\n"

    return header_content


# ── Main ─────────────────────────────────────────────────────────────────


def apply_patches(
    metal_cpp_dir: Path,
    config: dict,
    sdk_path: Path,
    strict: bool = False,
) -> None:
    """Apply all patches from config to the metal-cpp directory."""
    # Build a shared TypeResolver seeded with built-in types.
    resolver = TypeResolver()
    resolver.register("NSStringEncoding", "NS::StringEncoding")

    for fw_name, fw_cfg in config["frameworks"].items():
        namespace = fw_cfg["namespace"]
        prefix = fw_cfg["prefix"]
        strip_prefix = fw_cfg.get("strip_prefix", "")

        log.info("Framework: %s", fw_name)

        # Pre-register types for cross-references.
        for objc_name in fw_cfg.get("amend_types", {}):
            cpp = strip_objc_prefix(objc_name, strip_prefix)
            resolver.register(objc_name, f"{namespace}::{cpp}")
        for objc_name, type_cfg in fw_cfg.get("add_types", {}).items():
            cpp = strip_objc_prefix(objc_name, strip_prefix)
            resolver.register(objc_name, f"{namespace}::{cpp}")

        # Forward declarations.
        fwd = fw_cfg.get("forward_declarations")
        if fwd:
            apply_forward_declarations(metal_cpp_dir, fw_name, namespace, fwd)

        # Add new types (before amend, so new types exist for cross-refs).
        has_typed_consts = False
        for objc_name, type_cfg in fw_cfg.get("add_types", {}).items():
            log.info("  Add: %s", objc_name)
            if process_add_type(
                metal_cpp_dir,
                sdk_path,
                fw_name,
                namespace,
                prefix,
                strip_prefix,
                objc_name,
                type_cfg or {},
                resolver,
            ):
                has_typed_consts = True

        # Typed string constants need a dlsym fallback for older SDKs.
        if has_typed_consts:
            apply_loadsymbol_fallback(metal_cpp_dir, fw_name, namespace, prefix)

        # Amend existing types.
        for objc_name, amendments in fw_cfg.get("amend_types", {}).items():
            log.info("  Amend: %s", objc_name)

            if "move_to_public" in amendments:
                apply_move_to_public(
                    metal_cpp_dir,
                    fw_name,
                    strip_prefix,
                    objc_name,
                    amendments["move_to_public"],
                )

            if "base_class" in amendments:
                apply_base_class(
                    metal_cpp_dir,
                    fw_name,
                    prefix,
                    strip_prefix,
                    objc_name,
                    amendments["base_class"],
                )

            if "add_properties" in amendments:
                process_amend_properties(
                    metal_cpp_dir,
                    sdk_path,
                    fw_name,
                    namespace,
                    prefix,
                    strip_prefix,
                    objc_name,
                    amendments["add_properties"],
                    resolver,
                )

            if "add_methods" in amendments:
                process_amend_methods(
                    metal_cpp_dir,
                    sdk_path,
                    fw_name,
                    namespace,
                    prefix,
                    strip_prefix,
                    objc_name,
                    amendments["add_methods"],
                    resolver,
                )

    # Report unresolvable types.
    if resolver.unresolved:
        log.warning("Unresolvable types:")
        for typ, ctx in resolver.unresolved:
            log.warning("  %s (in %s)", typ, ctx)
        if strict:
            sys.exit(1)


def generate_patch(zip_path: Path, config: dict, sdk_path: Path) -> str:
    """Generate a unified patch by diffing original vs patched metal-cpp."""
    with tempfile.TemporaryDirectory() as tmpdir:
        original = Path(tmpdir) / "original"
        patched = Path(tmpdir) / "patched"
        original.mkdir()
        patched.mkdir()

        extract_zip(zip_path, original)
        extract_zip(zip_path, patched)
        apply_patches(patched, config, sdk_path)

        result = subprocess.run(
            ["diff", "-ruN", "original", "patched"],
            capture_output=True,
            text=True,
            cwd=tmpdir,
        )
        return result.stdout


def main() -> None:
    parser = argparse.ArgumentParser(description="Patch metal-cpp with Godot-specific amendments")
    parser.add_argument("metal_cpp_dir", nargs="?", type=Path, help="Path to metal-cpp root directory")
    parser.add_argument(
        "--zip",
        type=Path,
        help="Extract upstream metal-cpp zip into metal_cpp_dir before patching",
    )
    parser.add_argument(
        "--generate-patch",
        type=Path,
        metavar="ZIP",
        help="Generate a .patch file from a base zip (writes to stdout)",
    )
    parser.add_argument(
        "--config",
        type=Path,
        default=TOOL_DIR / "patches.yaml",
        help="Path to patches YAML config (default: patches.yaml next to this script)",
    )
    parser.add_argument("--sdk", default="macosx", help="xcrun SDK name (default: macosx)")
    parser.add_argument("--strict", action="store_true", help="Fail on unresolvable types")
    parser.add_argument("-v", "--verbose", action="store_true")
    args = parser.parse_args()

    logging.basicConfig(
        level=logging.DEBUG if args.verbose else logging.INFO,
        format="%(levelname)s: %(message)s",
        stream=sys.stderr,
    )

    config = load_config(args.config)
    sdk_path = get_sdk_path(args.sdk)
    log.info("SDK path: %s", sdk_path)

    if args.generate_patch:
        patch = generate_patch(args.generate_patch.resolve(), config, sdk_path)
        sys.stdout.write(patch)
    else:
        if not args.metal_cpp_dir:
            parser.error("metal_cpp_dir is required when not using --generate-patch")
        metal_cpp_dir = args.metal_cpp_dir.resolve()
        if args.zip:
            extract_zip(args.zip.resolve(), metal_cpp_dir)
        apply_patches(metal_cpp_dir, config, sdk_path, strict=args.strict)

    log.info("Done.")


if __name__ == "__main__":
    main()
