# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('set(GODOT_EDITOR_GDSCRIPT_TEMPLATES_SOURCE')
#   for file in list_files(cog, 'editor/script_templates', exts=['gd'], recursive=True):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
set(GODOT_EDITOR_GDSCRIPT_TEMPLATES_SOURCE
        editor/script_templates/CharacterBody2D/basic_movement.gd
        editor/script_templates/CharacterBody3D/basic_movement.gd
        editor/script_templates/EditorPlugin/plugin.gd
        editor/script_templates/EditorScenePostImport/basic_import_script.gd
        editor/script_templates/EditorScenePostImport/no_comments.gd
        editor/script_templates/EditorScript/basic_editor_script.gd
        editor/script_templates/Node/default.gd
        editor/script_templates/Object/empty.gd
        editor/script_templates/RichTextEffect/default.gd
        editor/script_templates/VisualShaderNodeCustom/basic.gd
)
# [[[end]]]

set(GENERATED_EDITOR_SCRIPT_TEMPLATES_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/editor/script_templates/templates.gen.h)
generate_file(make_editor_gdscript_templates "${GODOT_EDITOR_GDSCRIPT_TEMPLATES_SOURCE}" "${GENERATED_EDITOR_SCRIPT_TEMPLATES_OUTPUT}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

godot_add_library(godot-module-gdscript FOLDER modules STATIC
        ${GENERATED_EDITOR_SCRIPT_TEMPLATES_OUTPUT})
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-gdscript PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-gdscript PRIVATE
        gdscript.cpp
        gdscript.h
        gdscript_analyzer.cpp
        gdscript_analyzer.h
        gdscript_byte_codegen.cpp
        gdscript_byte_codegen.h
        gdscript_cache.cpp
        gdscript_cache.h
        gdscript_codegen.h
        gdscript_compiler.cpp
        gdscript_compiler.h
        gdscript_disassembler.cpp
        gdscript_editor.cpp
        gdscript_function.cpp
        gdscript_function.h
        gdscript_lambda_callable.cpp
        gdscript_lambda_callable.h
        gdscript_parser.cpp
        gdscript_parser.h
        gdscript_rpc_callable.cpp
        gdscript_rpc_callable.h
        gdscript_tokenizer.cpp
        gdscript_tokenizer.h
        gdscript_utility_functions.cpp
        gdscript_utility_functions.h
        gdscript_vm.cpp
        gdscript_warning.cpp
        gdscript_warning.h
        register_types.cpp
        register_types.h
)
# [[[end]]]

if (WANT_EDITOR)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(godot-module-gdscript PRIVATE')
    #   for file in list_files(cog, ['editor', 'language_server']):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(godot-module-gdscript PRIVATE
            editor/gdscript_docgen.cpp
            editor/gdscript_docgen.h
            editor/gdscript_highlighter.cpp
            editor/gdscript_highlighter.h
            editor/gdscript_translation_parser_plugin.cpp
            editor/gdscript_translation_parser_plugin.h
            language_server/gdscript_extend_parser.cpp
            language_server/gdscript_extend_parser.h
            language_server/gdscript_language_protocol.cpp
            language_server/gdscript_language_protocol.h
            language_server/gdscript_language_server.cpp
            language_server/gdscript_language_server.h
            language_server/gdscript_text_document.cpp
            language_server/gdscript_text_document.h
            language_server/gdscript_workspace.cpp
            language_server/gdscript_workspace.h
            language_server/godot_lsp.h
    )
    # [[[end]]]
endif ()
target_link_libraries(godot-module-gdscript PUBLIC godot-object minizip)
set_property(TARGET godot-module-gdscript PROPERTY CXX_STANDARD 17)
if (MSVC)
    # This is a very unfortunate workaround -- in release, this file essentially hangs compilation with MSVC.
    set_source_files_properties(gdscript_vm.cpp PROPERTIES COMPILE_FLAGS /Od)
endif()

set(GODOT_MODULE_GDSCRIPT_RESOURCES
        # doc_classes/GDScript.xml
        # doc_classes/@GDScript.xml
        icons/GDScript.svg
        icons/GDScriptInternal.svg
)
