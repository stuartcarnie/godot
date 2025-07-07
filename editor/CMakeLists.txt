godot_add_library(editor STATIC)

add_subdirectory(animation)
add_subdirectory(asset_library)
add_subdirectory(audio)
add_subdirectory(debugger)
add_subdirectory(doc)
add_subdirectory(docks)
add_subdirectory(export)
add_subdirectory(file_system)
add_subdirectory(gui)
add_subdirectory(icons)
add_subdirectory(import)
add_subdirectory(inspector)
add_subdirectory(plugins)
add_subdirectory(project_manager)
add_subdirectory(project_upgrade)
add_subdirectory(run)
add_subdirectory(scene)
add_subdirectory(script)
add_subdirectory(settings)
add_subdirectory(shader)
add_subdirectory(themes)
add_subdirectory(translations)
add_subdirectory(version_control)

# platform exporters

set(GENERATED_PLATFORM_EXPORTERS_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/editor/register_exporters.gen.cpp)
set(EXPORT_PLATFORM_SOURCE_PLATFORMS)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND EXPORT_PLATFORM_SOURCE_PLATFORMS "windows")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    list(APPEND EXPORT_PLATFORM_SOURCE_PLATFORMS "macos")
    list(APPEND EXPORT_PLATFORM_SOURCE_PLATFORMS "ios")
    list(APPEND EXPORT_PLATFORM_SOURCE_PLATFORMS "visionos")
elseif (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    list(APPEND EXPORT_PLATFORM_SOURCE_PLATFORMS "ios")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND EXPORT_PLATFORM_SOURCE_PLATFORMS "linuxbsd")
else ()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif ()

generate_godot_editor_platform_exporters("${EXPORT_PLATFORM_SOURCE_PLATFORMS}" "${GENERATED_PLATFORM_EXPORTERS_OUTPUT}")

set(GENERATED_EDITOR_DOC_DATA_CLASSES_PATH ${CMAKE_BINARY_INCLUDE_DIR}/editor/doc/doc_data_class_path.gen.h)
generate_document_class_paths("${GENERATED_EDITOR_DOC_DATA_CLASSES_PATH}")

target_sources(editor
        PRIVATE
        ${GODOT_EDITOR_PROPERTIES_TRANSLATIONS_OUTPUT}
        ${GODOT_EDITOR_TRANSLATIONS_DOCUMENTATION_OUTPUT}
        ${GENERATED_PLATFORM_EXPORTERS_OUTPUT}
        ${GENERATED_EDITOR_DOC_DATA_CLASSES_PATH}
)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        editor_data.cpp
        editor_data.h
        editor_interface.cpp
        editor_interface.h
        editor_log.cpp
        editor_log.h
        editor_main_screen.cpp
        editor_main_screen.h
        editor_node.cpp
        editor_node.h
        editor_string_names.h
        editor_undo_redo_manager.cpp
        editor_undo_redo_manager.h
        register_editor_types.cpp
        register_editor_types.h
)
# [[[end]]]

target_link_libraries(editor PUBLIC core minizip)
target_link_libraries(editor PRIVATE godot-doc)
target_include_directories(editor PRIVATE
        ${CMAKE_BINARY_INCLUDE_DIR}/editor
        ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(editor PRIVATE
        $<$<BOOL:${WANT_ENGINE_UPDATE_CHECK}>:ENGINE_UPDATE_CHECK_ENABLED>
)
set_property(TARGET editor PROPERTY CXX_STANDARD 17)

group_target_sources(editor)
