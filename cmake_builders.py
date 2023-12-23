#!/usr/local/bin/python3

import os
import sys
import pathlib
import argparse
import json

# Engine Modules
from glsl_builders import *
from gles3_builders import *
from core.core_builders import *
from methods import *
from core.input.input_builders import *
import core.extension.make_interface_dumper
import core.extension.make_wrappers

import core.object.make_virtuals
from main.main_builders import *
from scene.theme.icons.default_theme_icons_builders import *
from editor.editor_builders import *
import editor.themes.editor_theme_builders
from scene.theme.default_theme_builders import *
from editor.icons.editor_icons_builders import *
import editor.template_builders
from modules.modules_builders import generate_modules_enabled
from modules.text_server_adv.gdextension_build.methods import make_icu_data
import glob as Glob
import logging

logging.basicConfig(level=logging.INFO, format='${name}: ${message}', style='$')
log = logging.getLogger(os.path.basename(sys.argv[0]))


# region SCons emulation

class Target:
    def __init__(self, path, abspath=None):
        self._path = path
        self._abspath = path if abspath is None else abspath

    @property
    def path(self):
        return self._path

    @property
    def abspath(self):
        return self._abspath

    def srcnode(self):
        return self


class Environment:
    def __init__(self):
        self['doc_class_path'] = OrderedDict()
        self['module_dependencies'] = OrderedDict()
        self['module_icons_paths'] = list()
        pass

    def __getitem__(self, item):
        return self.__dict__[item]

    def __setitem__(self, key, value):
        self.__dict__[key] = value

    def module_check_dependencies(self, name: str) -> bool:
        return True

    @property
    def doc_class_path(self) -> OrderedDict:
        return self.__dict__['doc_class_path']

    @property
    def module_icons_paths(self) -> list:
        return self.__dict__['module_icons_paths']

    @property
    def editor_build(self) -> bool:
        return self.__dict__['editor_build']

    @property
    def module_dependencies(self) -> OrderedDict:
        return self.__dict__['module_dependencies']


# endregion

env = Environment()

env.disabled_modules = []
env.module_version_string = ""

env.__class__.add_module_version_string = add_module_version_string
env.__class__.module_add_dependencies = module_add_dependencies

env["platform"] = "macos"
env["arch"] = "arm64"
env["modules_enabled_by_default"] = True

# fake options
env["disable_3d"] = False
env["openxr"] = True
env["opengl3"] = True
env["minizip"] = True
env["builtin_certs"] = True
env["system_certs_path"] = ""


def detect_modules_within_searchpath(path: str, env: Environment, selected_platform: str) -> OrderedDict:
    # Built-in modules don't have nested modules,
    # so save the time it takes to parse directories.
    modules_detected = detect_modules(path, recursive=False)

    # Add module options.
    for name, path in modules_detected.items():
        sys.path.insert(0, path)
        import config

        if env["modules_enabled_by_default"]:
            enabled = True
            try:
                enabled = config.is_enabled()
            except AttributeError:
                pass
        else:
            enabled = False

        # Add module-specific options.
        # try:
        #     for opt in config.get_opts(selected_platform):
        #         opts.Add(opt)
        # except AttributeError:
        #     pass

        sys.path.remove(path)
        sys.modules.pop("config")

        env[f"module_{name}_enabled"] = enabled

    modules_enabled = OrderedDict()

    for name, path in modules_detected.items():
        if not env[f"module_{name}_enabled"]:
            continue
        sys.path.insert(0, path)
        env.current_module = name
        import config

        if config.can_build(env, selected_platform):
            # Disable it if a required dependency is missing.
            if not env.module_check_dependencies(name):
                continue

            config.configure(env)
            # Get doc classes paths (if present)
            try:
                doc_classes = config.get_doc_classes()
                doc_path = config.get_doc_path()
                for c in doc_classes:
                    env.doc_class_path[c] = path + "/" + doc_path
            except Exception:
                pass
            # Get icon paths (if present)
            try:
                icons_path = config.get_icons_path()
                env.module_icons_paths.append(path + "/" + icons_path)
            except Exception:
                # Default path for module icons
                env.module_icons_paths.append(path + "/" + "icons")
            modules_enabled[name] = path

        sys.path.remove(path)
        sys.modules.pop("config")

    return modules_enabled


def check_output(func):
    def wrapper(args: argparse.Namespace):
        output_path = args.output
        output_dir = pathlib.Path(output_path).parent

        if not output_dir.exists():
            log.debug(f'Creating directory: {output_dir.as_posix()}')
            try:
                output_dir.mkdir(parents=True, exist_ok=True)
            except:
                log.error(f'Unable to create directory: {output_dir.as_posix()}')
                return 1
        return func(args)

    return wrapper


def source_target(func):
    def wrapper(args: argparse.Namespace):
        has_i1 = hasattr(args, 'input')
        has_i2 = hasattr(args, 'input2')
        has_o1 = hasattr(args, 'output')
        has_o2 = hasattr(args, 'output2')

        if has_i1 and has_i2 and has_o1 and has_o2:
            return func(args.input, args.input2, args.output, args.output2)
        elif has_i1 and has_i2 and has_o1:
            return func(args.input, args.input2, args.output)
        elif has_o1 and has_o2:
            return func(args.output, args.output2)
        elif has_i1 and has_o1:
            return func(args.input, args.output)
        elif has_o1:
            return func(args.output)
        else:
            raise Exception(f'Unrecognized arguments: {args}')

    return wrapper


@check_output
@source_target
def cmd_glsl(source: str, target: str) -> int:
    build_rd_header(source, target, None)
    return 0


@check_output
@source_target
def cmd_gles3(source: str, target: str) -> int:
    build_gles3_header(source, include="drivers/gles3/shader_gles3.h", class_suffix="GLES3",
                       optional_output_filename=target)
    return 0


@check_output
@source_target
def cmd_glsl_raw(source: str, target: str) -> int:
    build_raw_header(filename=source, optional_output_filename=target)
    return 0


@check_output
@source_target
def cmd_certs_header(source: str, target: str) -> int:
    make_certs_header(target=[target], source=[source], env=env)
    return 0


@check_output
@source_target
def cmd_authors_header(source: str, target: str) -> int:
    make_authors_header(target=[target], source=[source], env=None)
    return 0


@check_output
@source_target
def cmd_donors_header(source: str, target: str) -> int:
    make_donors_header(target=[target], source=[source], env=None)
    return 0


@check_output
def cmd_license_header(args: argparse.Namespace) -> int:
    make_license_header(target=[args.output], source=[args.input_copyright, args.input_license], env=None)
    return 0


@check_output
@source_target
def cmd_disabled_classes(target: str) -> int:
    write_disabled_classes(class_list=[], output_filepath=target)
    return 0


@check_output
@source_target
def cmd_controller_mappings(source: [str], target: str) -> int:
    make_default_controller_mappings(target=[target], source=source, env=None)
    return 0


@check_output
@source_target
def cmd_gdextension_interface_dumper(source: str, target: str) -> int:
    core.extension.make_interface_dumper.run(target=[target], source=[source], env=None)
    return 0


@check_output
@source_target
def cmd_make_app_icon(source: str, target: str) -> int:
    make_app_icon(target=[target], source=[source], env=None)
    return 0


@check_output
@source_target
def cmd_make_app_splash(source: str, target: str) -> int:
    make_splash(target=[target], source=[source], env=None)
    return 0


@check_output
@source_target
def cmd_resource_scene_make_fonts_header(source: str, target: str) -> int:
    make_fonts_header(target=[target], source=[source], env=None)
    return 0


@check_output
@source_target
def cmd_resource_make_default_theme_icons(source: [str], target: str) -> int:
    make_default_theme_icons_action(target=[target], source=source, env=None)
    return 0


@check_output
@source_target
def cmd_make_icu_data(source: str, target: str) -> int:
    make_icu_data(target=[Target(target)], source=[Target(source)], env=None)
    return 0


@check_output
@source_target
def cmd_godot_editor_builtin_fonts(source: str, target: str) -> int:
    source_input = source.split()
    make_fonts_header(target=[target], source=source_input, env=None)
    return 0


def read_lines(source: str) -> [str]:
    with open(source, 'r') as file:
        source_input = file.readlines()

    return [line.rstrip('\n') for line in source_input]


@check_output
@source_target
def cmd_make_documentation_header_compressed(source: str, target: str) -> int:
    source_input = read_lines(source)
    make_doc_header(target=[target], source=source_input, env=None)
    return 0


@check_output
@source_target
def cmd_make_editor_icons_action(source: str, target: str) -> int:
    source_input = read_lines(source)
    make_editor_icons_action(target=[target], source=source_input, env=None)
    return 0


@check_output
@source_target
def cmd_make_editor_translations(source: [str], target: str) -> int:
    make_editor_translations_header(target=[target], source=source, env=None)
    return 0


@check_output
@source_target
def cmd_make_editor_properties_translations(source: [str], target: str) -> int:
    make_property_translations_header(target=[target], source=source, env=None)
    return 0


@check_output
@source_target
def cmd_make_editor_documentation_translations(source: [str], target: str) -> int:
    make_doc_translations_header(target=[target], source=source, env=None)
    return 0


@check_output
@source_target
def cmd_make_editor_themes_fonts(source: str, target: str) -> int:
    source_input = read_lines(source)
    editor.themes.editor_theme_builders.make_fonts_header(target=[target], source=source_input, env=None)
    return 0


@check_output
@source_target
def cmd_make_version_data_headers(target: str, target2: str) -> int:
    generate_version_header(module_version_string="", optional_version_outpath=target,
                            optional_version_hash_output=target2)
    return 0


@check_output
@source_target
def cmd_make_script_encryption_header(target: str) -> int:
    write_script_encryption_key(target=target)
    return 0


@check_output
@source_target
def cmd_make_extension_wrapper(target: str) -> int:
    core.extension.make_wrappers.run(target=[target], source=None, env=None)
    return 0


@check_output
@source_target
def cmd_make_gdscript_virtuals(target: str) -> int:
    core.object.make_virtuals.run(target=[target], source=None, env=None)
    return 0


@check_output
@source_target
def cmd_make_editor_gdscript_templates(source: [str], target: str) -> int:
    editor.template_builders.make_templates(target=[target], source=source, env=None)
    return 0


@check_output
@source_target
def cmd_make_register_platform_apis(target: str) -> int:
    def make_platform_apis(target, platforms):
        env.platform_sources = []

        # Register platform-exclusive APIs
        reg_apis_inc = '#include "register_platform_apis.h"\n'
        reg_apis = "void register_platform_apis() {\n"
        unreg_apis = "void unregister_platform_apis() {\n"
        for platform in platforms:
            reg_apis += "\tregister_" + platform + "_api();\n"
            unreg_apis += "\tunregister_" + platform + "_api();\n"
            reg_apis_inc += '#include "' + platform + '/api/api.h"\n'
        reg_apis_inc += "\n"
        reg_apis += "}\n\n"
        unreg_apis += "}\n"

        # NOTE: It is safe to generate this file here, since this is still execute serially
        with open(target, "w", encoding="utf-8") as f:
            f.write(reg_apis_inc)
            f.write(reg_apis)
            f.write(unreg_apis)

    make_platform_apis(target=target, platforms=[])
    return 0


@check_output
@source_target
def cmd_make_editor_platform_exporters(source: str, target: str) -> int:
    def make_platform_exporters_register(target, platform_exporters):
        # Register exporters
        reg_exporters_inc = '#include "register_exporters.h"\n\n'
        reg_exporters = "void register_exporters() {\n"
        for e in platform_exporters:
            reg_exporters += "\tregister_" + e + "_exporter();\n"
            reg_exporters_inc += '#include "platform/' + e + '/export/export.h"\n'
        reg_exporters += "}\n\n"
        reg_exporters += "void register_exporter_types() {\n"
        for e in platform_exporters:
            reg_exporters += "\tregister_" + e + "_exporter_types();\n"
        reg_exporters += "}\n"

        # NOTE: It is safe to generate this file here, since this is still executed serially
        with open(target, "w", encoding="utf-8") as f:
            f.write(reg_exporters_inc)
            f.write(reg_exporters)

    # TODO(sgc): need to handle all exporters
    platform_exporters_input = source.split()
    make_platform_exporters_register(target=target, platform_exporters=platform_exporters_input)
    return 0


@check_output
def cmd_make_modules_enabled_and_types(args: argparse.Namespace) -> int:
    input_file = args.input
    log.debug('Detect all the Godot Modules to generate the module data')
    base_godot_engine_dir = str(pathlib.Path(input_file).parent) + "/"
    unordered_modules = detect_modules_within_searchpath(input_file, env, env['platform'])
    # Strip paths based on the output
    for dict_key, dict_item in unordered_modules.items():
        unordered_modules[dict_key] = dict_item.replace(base_godot_engine_dir, "")

    # If a custom path is provided apply it
    if args.input2:
        custom_modules = detect_modules_within_searchpath(args.input2, env, env['platform'])
        unordered_modules.update(custom_modules)

    modules = OrderedDict()
    for key, value in sorted(unordered_modules.items()):
        modules[key] = value

    original_cwd = os.getcwd()
    os.chdir(base_godot_engine_dir)

    env['module_list'] = modules
    sort_module_list(env)

    # Write out the results
    generate_modules_enabled(target=[Target(str(args.output2))], source=modules.keys(), env=env)
    write_modules(modules, cpp_path=str(args.output))

    os.chdir(original_cwd)

    return 0


@check_output
@source_target
def cmd_make_data_class_path(source: str, target: str) -> int:
    modules = detect_modules_within_searchpath(source + "modules/", env, env['platform'])

    # Push the current working directory
    original_cwd = os.getcwd()
    base_godot_engine_dir = str(pathlib.Path(source).parent) + "/"
    os.chdir(base_godot_engine_dir)

    def generate_docs_from_path(base_directory, relative_base_path):
        log.debug("base_directory: %s   %s " % (base_directory, relative_base_path))
        found_docs = {}
        search_string = base_directory + "*.xml"
        found_xml_doc_files = Glob.glob(search_string, recursive=True)

        relative_doc_class_directory = base_directory.replace(relative_base_path, "")
        relative_doc_class_directory = relative_doc_class_directory[:-1]

        for current_xml_doc in found_xml_doc_files:
            current_doc_module = str(pathlib.Path(current_xml_doc).stem)
            found_docs[current_doc_module] = relative_doc_class_directory

        return found_docs

    # source: editor/SCsub
    def make_doc_data_class_path(to_path, doc_class_path):
        # NOTE: It is safe to generate this file here, since this is still executed serially
        g = open(to_path, "w", encoding="utf-8")
        g.write("static const int _doc_data_class_path_count = " + str(len(doc_class_path)) + ";\n")
        g.write("struct _DocDataClassPath { const char* name; const char* path; };\n")

        g.write(
            "static const _DocDataClassPath _doc_data_class_paths[" + str(len(doc_class_path) + 1) + "] = {\n")
        for c in sorted(doc_class_path):
            g.write('\t{"' + c + '", "' + doc_class_path[c] + '"},\n')
        g.write("\t{nullptr, nullptr}\n")
        g.write("};\n")

        g.close()

    docs = {}
    modules_relative_base_directory = source
    for d in modules:
        modules_base_directory = ((source + "modules/") + d + "/doc_classes/")
        new_entries = generate_docs_from_path(modules_base_directory, modules_relative_base_directory)
        docs.update(new_entries)

    make_doc_data_class_path(to_path=target, doc_class_path=docs)

    # Pop it back
    os.chdir(original_cwd)
    return 0


@check_output
def cmd_generate_export_icon(args: argparse.Namespace) -> int:
    # From platform_methods.py
    def generate_export_icons(platform_name: str, icon_type: str, svg_path: str, svg_gen_path: str):
        """
        Generate headers for logo and run icon for the export plugin.
        """
        svgf = open(svg_path, "rb")
        b = svgf.read(1)
        svg_str = " /* AUTOGENERATED FILE, DO NOT EDIT */ \n"
        svg_str += " static const char *_" + platform_name + "_" + icon_type + '_svg = "'
        while len(b) == 1:
            svg_str += "\\" + hex(ord(b))[1:]
            b = svgf.read(1)

        svg_str += '";\n'

        svgf.close()

        # NOTE: It is safe to generate this file here, since this is still executed serially.
        with open(svg_gen_path, "w") as svgw:
            svgw.write(svg_str)

    generate_export_icons(args.platform_name, args.icon_type, args.input, args.output)
    return 0


def _main() -> int:
    # Command accepts exactly one input and one output path
    args_in_out = argparse.ArgumentParser(add_help=False)
    args_in_out.add_argument('--input', dest='input', required=True)
    args_in_out.add_argument('--output', dest='output', required=True)

    # --input is type: [str]
    args_inl_out = argparse.ArgumentParser(add_help=False)
    args_inl_out.add_argument('--input', nargs='*', dest='input', required=True)
    args_inl_out.add_argument('--output', dest='output', required=True)

    # Command requires two input and one output
    args_in2_out = argparse.ArgumentParser(add_help=False)
    args_in2_out.add_argument('--input', dest='input', required=True)
    args_in2_out.add_argument('--input2', dest='input2', required=True)
    args_in2_out.add_argument('--output', dest='output', required=True)

    # Command requires a single output
    args_out = argparse.ArgumentParser(add_help=False)
    args_out.add_argument('--output', dest='output', required=True)

    # Command requires two outputs
    args_out2 = argparse.ArgumentParser(add_help=False)
    args_out2.add_argument('--output', dest='output', required=True)
    args_out2.add_argument('--output2', dest='output2', required=True)

    parser = argparse.ArgumentParser()
    sp = parser.add_subparsers(dest='command', help='command help', required=True)

    sp.add_parser('glsl', parents=[args_in_out]).set_defaults(func=cmd_glsl)
    sp.add_parser('gles3', parents=[args_in_out]).set_defaults(func=cmd_gles3)
    sp.add_parser('glsl_raw', parents=[args_in_out]).set_defaults(func=cmd_glsl_raw)
    sp.add_parser('certs_header', parents=[args_in_out]).set_defaults(func=cmd_certs_header)
    sp.add_parser('authors_header', parents=[args_in_out]).set_defaults(func=cmd_authors_header)
    sp.add_parser('donors_header', parents=[args_in_out]).set_defaults(func=cmd_donors_header)

    cmd = sp.add_parser('license_header')
    cmd.add_argument('--input-copyright', dest='input_copyright', required=True)
    cmd.add_argument('--input-license', dest='input_license', required=True)
    cmd.add_argument('--output', dest='output', required=True)
    cmd.set_defaults(func=cmd_license_header)

    sp.add_parser('disabled_classes', parents=[args_out]).set_defaults(func=cmd_disabled_classes)
    sp.add_parser('controller_mappings', parents=[args_inl_out]).set_defaults(func=cmd_controller_mappings)
    sp.add_parser('gdextension_interface_dumper', parents=[args_in_out]).set_defaults(
        func=cmd_gdextension_interface_dumper)
    sp.add_parser('make_app_icon', parents=[args_in_out]).set_defaults(func=cmd_make_app_icon)
    sp.add_parser('make_app_splash', parents=[args_in_out]).set_defaults(func=cmd_make_app_splash)
    sp.add_parser('resource_scene_make_fonts_header', parents=[args_in_out]).set_defaults(
        func=cmd_resource_scene_make_fonts_header)
    sp.add_parser('resource_make_default_theme_icons', parents=[args_inl_out]).set_defaults(
        func=cmd_resource_make_default_theme_icons)
    sp.add_parser('make_icu_data', parents=[args_in_out]).set_defaults(func=cmd_make_icu_data)
    sp.add_parser('godot_editor_builtin_fonts', parents=[args_in_out]).set_defaults(func=cmd_godot_editor_builtin_fonts)
    sp.add_parser('make_documentation_header_compressed', parents=[args_in_out]).set_defaults(
        func=cmd_make_documentation_header_compressed)
    sp.add_parser('make_editor_icons_action', parents=[args_in_out]).set_defaults(func=cmd_make_editor_icons_action)
    sp.add_parser('make_editor_translations', parents=[args_inl_out]).set_defaults(func=cmd_make_editor_translations)
    sp.add_parser('make_editor_properties_translations', parents=[args_inl_out]).set_defaults(
        func=cmd_make_editor_properties_translations)
    sp.add_parser('make_editor_documentation_translations', parents=[args_inl_out]).set_defaults(
        func=cmd_make_editor_documentation_translations)
    sp.add_parser('make_editor_themes_fonts', parents=[args_in_out]).set_defaults(
        func=cmd_make_editor_themes_fonts)
    sp.add_parser('make_version_data_headers', parents=[args_out2]).set_defaults(func=cmd_make_version_data_headers)
    sp.add_parser('make_script_encryption_header', parents=[args_out]).set_defaults(
        func=cmd_make_script_encryption_header)
    sp.add_parser('make_extension_wrapper', parents=[args_out]).set_defaults(func=cmd_make_extension_wrapper)
    sp.add_parser('make_gdscript_virtuals', parents=[args_out]).set_defaults(func=cmd_make_gdscript_virtuals)
    sp.add_parser('make_editor_gdscript_templates', parents=[args_inl_out]).set_defaults(
        func=cmd_make_editor_gdscript_templates)
    sp.add_parser('make_register_platform_apis', parents=[args_out]).set_defaults(
        func=cmd_make_register_platform_apis)
    sp.add_parser('make_editor_platform_exporters', parents=[args_in_out]).set_defaults(
        func=cmd_make_editor_platform_exporters)

    cmd = sp.add_parser('make_modules_enabled_and_types')
    cmd.add_argument('--input', dest='input', required=True)
    cmd.add_argument('--input2', dest='input2', required=False)
    cmd.add_argument('--output', dest='output', required=True)
    cmd.add_argument('--output2', dest='output2', required=True)
    cmd.set_defaults(func=cmd_make_modules_enabled_and_types)

    sp.add_parser('make_data_class_path', parents=[args_in_out]).set_defaults(func=cmd_make_data_class_path)

    cmd = sp.add_parser('generate_export_icon')
    cmd.add_argument('--platform-name', dest='platform_name', required=True)
    cmd.add_argument('--icon-type', dest='icon_type', required=True)
    cmd.add_argument('--input', dest='input', required=True)
    cmd.add_argument('--output', dest='output', required=True)
    cmd.set_defaults(func=cmd_generate_export_icon)

    parser.add_argument("--env", dest="env", required=True)

    parsed = parser.parse_args()

    for arg in [var for var in vars(parsed) if var != 'func']:
        log.debug(f'{arg} : {getattr(parsed, arg)}')

    # Load the environment
    # Read JSON data from a file
    with open(parsed.env, 'r') as file:
        cmake_env = json.load(file)

    # Set the environment
    for key, value in cmake_env.items():
        env[key] = value

    if hasattr(parsed, 'func'):
        return parsed.func(parsed)
    return -1


# region Helper functions

# Generate AES256 script encryption key

def write_script_encryption_key(target):
    txt = "0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0"
    if "SCRIPT_AES256_ENCRYPTION_KEY" in os.environ:
        key = os.environ["SCRIPT_AES256_ENCRYPTION_KEY"]
        ec_valid = True
        if len(key) != 64:
            ec_valid = False
        else:
            txt = ""
            for i in range(len(key) >> 1):
                if i > 0:
                    txt += ","
                txts = "0x" + key[i * 2: i * 2 + 2]
                try:
                    int(txts, 16)
                except Exception:
                    ec_valid = False
                txt += txts
        if not ec_valid:
            print("Error: Invalid AES256 encryption key, not 64 hexadecimal characters: '" + key + "'.")
            print(
                "Unset 'SCRIPT_AES256_ENCRYPTION_KEY' in your environment "
                "or make sure that it contains exactly 64 hexadecimal characters."
            )
            sys.exit(255)

    # NOTE: It is safe to generate this file here, since this is still executed serially
    with open(target, "w") as f:
        f.write('#include "core/config/project_settings.h"\nuint8_t script_encryption_key[32]={' + txt + "};\n")


# endregion

# region cog helpers

def list_files(cog, search_paths: str | list[str], exts: [str] = None, recursive: bool = False, all_files=False) -> [
    str]:
    base_path = pathlib.Path(cog.inFile).parent

    search_paths = [search_paths] if isinstance(search_paths, str) else search_paths

    exts = ['cpp', 'h', 'hpp', 'mm', 'm', 'c', 'cc', 'cxx'] if exts is None else exts
    exts = [f'{ext}' if ext.startswith('.') else f'.{ext}' for ext in exts]

    res = []
    for search_path in search_paths:
        for root, dirs, files in os.walk(base_path.joinpath(search_path)):
            if not recursive: dirs.clear()
            root = pathlib.Path(root)
            for f in [root.joinpath(file).relative_to(base_path) for file in files]:
                if f.suffix in exts or all_files:
                    res.append(f)

    res.sort()
    return res


# endregion

if __name__ == '__main__':
    if sys.version_info.major < 3:
        log.error(f'Script requires Python3')

    sys.exit(_main())
