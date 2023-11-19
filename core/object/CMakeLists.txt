set(GODOT_CONFIG_VERSION_HEADER_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/core/version_generated.gen.h)
set(GODOT_CONFIG_VERSION_HASH_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/core/version_hash.gen.cpp)
generate_version_information(${GODOT_CONFIG_VERSION_HEADER_OUTPUT} ${GODOT_CONFIG_VERSION_HASH_OUTPUT})

set(GODOT_INTERFACE_WRAPPERS_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/core/extension/ext_wrappers.gen.inc)
generate_godot_extension_wrappers(${GODOT_INTERFACE_WRAPPERS_OUTPUT})

set(GODOT_OBJECT_GD_VIRTUALS_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/core/object/gdvirtual.gen.inc)
generate_godot_gdscript_virtuals(${GODOT_OBJECT_GD_VIRTUALS_OUTPUT})

set(GODOT_CORE_OBJECT_DISABLED_CLASSES_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/core/disabled_classes.gen.h)
generate_core_disabled_classes("None" ${GODOT_CORE_OBJECT_DISABLED_CLASSES_OUTPUT})

# Modules to enable
set(GENERATED_MODULES_ENABLED_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/modules/modules_enabled.gen.h)
set(GENERATED_MODULES_REGISTER_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/modules/register_module_types.gen.cpp)
set(GODOT_CUSTOM_ENGINE_MODULES_DIRECTORY "")
generate_enabled_modules_and_register("${GODOT_ENGINE_ROOT_DIRECTORY}/modules/" "${GODOT_CUSTOM_ENGINE_MODULES_DIRECTORY}" "${GENERATED_MODULES_ENABLED_OUTPUT}" "${GENERATED_MODULES_REGISTER_OUTPUT}")

godot_add_library(godot-object FOLDER core STATIC
        ${GODOT_CORE_OBJECT_DISABLED_CLASSES_OUTPUT}
        ${GODOT_INTERFACE_INTERFACE_DUMP_OUTPUT}
        ${GODOT_INTERFACE_WRAPPERS_OUTPUT}
        ${GODOT_OBJECT_GD_VIRTUALS_OUTPUT}
        ${GODOT_CONFIG_VERSION_HEADER_OUTPUT}
        ${GODOT_CONFIG_VERSION_HASH_OUTPUT}
        ${GENERATED_MODULES_ENABLED_OUTPUT}
        ${GENERATED_MODULES_REGISTER_OUTPUT})
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-object PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-object PRIVATE
        callable_method_pointer.cpp
        callable_method_pointer.h
        class_db.cpp
        class_db.h
        message_queue.cpp
        message_queue.h
        method_bind.cpp
        method_bind.h
        object.cpp
        object.h
        object_id.h
        ref_counted.cpp
        ref_counted.h
        script_instance.cpp
        script_instance.h
        script_language.cpp
        script_language.h
        script_language_extension.cpp
        script_language_extension.h
        undo_redo.cpp
        undo_redo.h
        worker_thread_pool.cpp
        worker_thread_pool.h
)
# [[[end]]]
target_include_directories(godot-object PUBLIC
        ${GODOT_ENGINE_ROOT_DIRECTORY}
        ${CMAKE_BINARY_INCLUDE_DIR})

target_include_directories(godot-object PRIVATE
        ${CMAKE_BINARY_INCLUDE_DIR}/modules
        ${GODOT_ENGINE_ROOT_DIRECTORY}/modules)

target_compile_definitions(godot-object PUBLIC
        $<${WANT_DEBUG_FEATURES}:DEBUG_ENABLED>)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_include_directories(godot-object PUBLIC ${GODOT_SOURCE_ROOT_PLATFORM_DIR}/windows)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_include_directories(godot-object PUBLIC ${GODOT_SOURCE_ROOT_PLATFORM_DIR}/macos)
else ()
    target_include_directories(godot-object PUBLIC ${GODOT_SOURCE_ROOT_PLATFORM_DIR}/linuxbsd)
endif ()
