cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

add_subdirectory(debugger)
add_subdirectory(export)
add_subdirectory(gui)
add_subdirectory(icons)
add_subdirectory(import)
add_subdirectory(plugins)
add_subdirectory(project_manager)
add_subdirectory(themes)
add_subdirectory(translations)

set(GENERATED_PLATFORM_EXPORTERS_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/editor/register_exporters.gen.cpp)
set(EXPORT_PLATFORM_SOURCE_PLATFORMS)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND EXPORT_PLATFORM_SOURCE_PLATFORMS "windows")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    list(APPEND EXPORT_PLATFORM_SOURCE_PLATFORMS "macos")
elseif (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    list(APPEND EXPORT_PLATFORM_SOURCE_PLATFORMS "ios")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND EXPORT_PLATFORM_SOURCE_PLATFORMS "linuxbsd")
else ()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif ()

generate_godot_editor_platform_exporters("${EXPORT_PLATFORM_SOURCE_PLATFORMS}" "${GENERATED_PLATFORM_EXPORTERS_OUTPUT}")

set(GENERATED_EDITOR_DOC_DATA_CLASSES_PATH ${CMAKE_BINARY_INCLUDE_DIR}/editor/doc_data_class_path.gen.h)
generate_document_class_paths("${GENERATED_EDITOR_DOC_DATA_CLASSES_PATH}")

godot_add_library(godot-editor FOLDER editor STATIC
        ${GODOT_EDITOR_PROPERTIES_TRANSLATIONS_OUTPUT}
        ${GODOT_EDITOR_TRANSLATIONS_DOCUMENTATION_OUTPUT}
        ${GENERATED_PLATFORM_EXPORTERS_OUTPUT}
        ${GENERATED_EDITOR_DOC_DATA_CLASSES_PATH})
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-editor PRIVATE
        action_map_editor.cpp
        action_map_editor.h
        animation_bezier_editor.cpp
        animation_bezier_editor.h
        animation_track_editor.cpp
        animation_track_editor.h
        animation_track_editor_plugins.cpp
        animation_track_editor_plugins.h
        audio_stream_preview.cpp
        audio_stream_preview.h
        code_editor.cpp
        code_editor.h
        connections_dialog.cpp
        connections_dialog.h
        create_dialog.cpp
        create_dialog.h
        dependency_editor.cpp
        dependency_editor.h
        directory_create_dialog.cpp
        directory_create_dialog.h
        doc_tools.cpp
        doc_tools.h
        editor_about.cpp
        editor_about.h
        editor_asset_installer.cpp
        editor_asset_installer.h
        editor_atlas_packer.cpp
        editor_atlas_packer.h
        editor_audio_buses.cpp
        editor_audio_buses.h
        editor_autoload_settings.cpp
        editor_autoload_settings.h
        editor_build_profile.cpp
        editor_build_profile.h
        editor_command_palette.cpp
        editor_command_palette.h
        editor_data.cpp
        editor_data.h
        editor_dock_manager.cpp
        editor_dock_manager.h
        editor_feature_profile.cpp
        editor_feature_profile.h
        editor_file_system.cpp
        editor_file_system.h
        editor_folding.cpp
        editor_folding.h
        editor_help.cpp
        editor_help.h
        editor_help_search.cpp
        editor_help_search.h
        editor_inspector.cpp
        editor_inspector.h
        editor_interface.cpp
        editor_interface.h
        editor_layouts_dialog.cpp
        editor_layouts_dialog.h
        editor_locale_dialog.cpp
        editor_locale_dialog.h
        editor_log.cpp
        editor_log.h
        editor_native_shader_source_visualizer.cpp
        editor_native_shader_source_visualizer.h
        editor_node.cpp
        editor_node.h
        editor_paths.cpp
        editor_paths.h
        editor_plugin.cpp
        editor_plugin.h
        editor_plugin_settings.cpp
        editor_plugin_settings.h
        editor_properties.cpp
        editor_properties.h
        editor_properties_array_dict.cpp
        editor_properties_array_dict.h
        editor_properties_vector.cpp
        editor_properties_vector.h
        editor_property_name_processor.cpp
        editor_property_name_processor.h
        editor_quick_open.cpp
        editor_quick_open.h
        editor_resource_picker.cpp
        editor_resource_picker.h
        editor_resource_preview.cpp
        editor_resource_preview.h
        editor_run.cpp
        editor_run.h
        editor_run_native.cpp
        editor_run_native.h
        editor_script.cpp
        editor_script.h
        editor_sectioned_inspector.cpp
        editor_sectioned_inspector.h
        editor_settings.cpp
        editor_settings.h
        editor_settings_dialog.cpp
        editor_settings_dialog.h
        editor_string_names.cpp
        editor_string_names.h
        editor_translation.cpp
        editor_translation.h
        editor_translation_parser.cpp
        editor_translation_parser.h
        editor_undo_redo_manager.cpp
        editor_undo_redo_manager.h
        editor_vcs_interface.cpp
        editor_vcs_interface.h
        engine_update_label.cpp
        engine_update_label.h
        event_listener_line_edit.cpp
        event_listener_line_edit.h
        fbx_importer_manager.cpp
        fbx_importer_manager.h
        filesystem_dock.cpp
        filesystem_dock.h
        find_in_files.cpp
        find_in_files.h
        group_settings_editor.cpp
        group_settings_editor.h
        groups_editor.cpp
        groups_editor.h
        history_dock.cpp
        history_dock.h
        import_defaults_editor.cpp
        import_defaults_editor.h
        import_dock.cpp
        import_dock.h
        input_event_configuration_dialog.cpp
        input_event_configuration_dialog.h
        inspector_dock.cpp
        inspector_dock.h
        localization_editor.cpp
        localization_editor.h
        multi_node_edit.cpp
        multi_node_edit.h
        node_dock.cpp
        node_dock.h
        plugin_config_dialog.cpp
        plugin_config_dialog.h
        pot_generator.cpp
        pot_generator.h
        progress_dialog.cpp
        progress_dialog.h
        project_converter_3_to_4.cpp
        project_converter_3_to_4.h
        project_manager.cpp
        project_manager.h
        project_settings_editor.cpp
        project_settings_editor.h
        property_selector.cpp
        property_selector.h
        register_editor_types.cpp
        register_editor_types.h
        register_exporters.h
        rename_dialog.cpp
        rename_dialog.h
        renames_map_3_to_4.cpp
        renames_map_3_to_4.h
        reparent_dialog.cpp
        reparent_dialog.h
        run_instances_dialog.cpp
        run_instances_dialog.h
        scene_create_dialog.cpp
        scene_create_dialog.h
        scene_tree_dock.cpp
        scene_tree_dock.h
        script_create_dialog.cpp
        script_create_dialog.h
        shader_create_dialog.cpp
        shader_create_dialog.h
        shader_globals_editor.cpp
        shader_globals_editor.h
        surface_upgrade_tool.cpp
        surface_upgrade_tool.h
        window_wrapper.cpp
        window_wrapper.h
)
# [[[end]]]

target_link_libraries(godot-editor PUBLIC godot-object godot-config minizip)
target_link_libraries(godot-editor PRIVATE godot-doc godot-editor-icons godot-editor-themes godot-editor-translations)
target_include_directories(godot-editor PRIVATE
        ${CMAKE_BINARY_INCLUDE_DIR}/editor
        ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(godot-editor PRIVATE
        $<$<BOOL:${WANT_ENGINE_UPDATE_CHECK}>:ENGINE_UPDATE_CHECK_ENABLED>
)
set_property(TARGET godot-editor PROPERTY CXX_STANDARD 17)

