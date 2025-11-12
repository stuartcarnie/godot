godot_add_library(core STATIC)

# Generated: object

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


# Generated: config

set(GODOT_CORE_AUTHORS_SOURCE ${GODOT_ENGINE_ROOT_DIRECTORY}/AUTHORS.md)
set(GODOT_CORE_AUTHORS_SOURCE_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/core/authors.gen.h)
generate_file(authors_header ${GODOT_CORE_AUTHORS_SOURCE} ${GODOT_CORE_AUTHORS_SOURCE_OUTPUT})

set(GODOT_CORE_DONORS_SOURCE ${GODOT_ENGINE_ROOT_DIRECTORY}/DONORS.md)
set(GODOT_CORE_DONORS_SOURCE_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/core/donors.gen.h)
generate_file(donors_header ${GODOT_CORE_DONORS_SOURCE} ${GODOT_CORE_DONORS_SOURCE_OUTPUT})

set(GODOT_CORE_CONFIG_COPYRIGHT_SOURCE ${GODOT_ENGINE_ROOT_DIRECTORY}/COPYRIGHT.txt)
set(GODOT_CORE_CONFIG_LICENSE_SOURCE ${GODOT_ENGINE_ROOT_DIRECTORY}/LICENSE.txt)
set(GODOT_CORE_CONFIG_LICENSE_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/core/license.gen.h)
generate_license_file(${GODOT_CORE_CONFIG_COPYRIGHT_SOURCE} ${GODOT_CORE_CONFIG_LICENSE_SOURCE} ${GODOT_CORE_CONFIG_LICENSE_OUTPUT})

# Generate the script encryption
set(GODOT_SCRIPT_ENCRYPTION_GENERATE_HEADER ${CMAKE_BINARY_INCLUDE_DIR}/core/script_encryption_key.gen.cpp)
generate_script_encryption_header("" ${GODOT_SCRIPT_ENCRYPTION_GENERATE_HEADER})

# Generated: input

set(GODOT_CORE_INPUT_GAME_CONTROLLER_MAPPINGS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/input/gamecontrollerdb.txt)
set(GODOT_CORE_INPUT_GODOT_CONTROLLER_MAPPINGS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/input/godotcontrollerdb.txt)
set(GODOT_CORE_INPUT_CONTROLLER_MAPPINGS_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/core/input/default_controller_mappings.gen.cpp)
generate_core_controller_mappings_sources(${GODOT_CORE_INPUT_GAME_CONTROLLER_MAPPINGS_SOURCE} ${GODOT_CORE_INPUT_GODOT_CONTROLLER_MAPPINGS_SOURCE} ${GODOT_CORE_INPUT_CONTROLLER_MAPPINGS_OUTPUT})

# Generated: io

set(GODOT_CORE_IO_CA_CERTIFICATES_SOURCE ${GODOT_SOURCE_ROOT_THIRDPARTY_DIR}/certs/ca-certificates.crt)
set(GODOT_CORE_IO_CA_CERTIFICATES_DESTINATION ${CMAKE_BINARY_INCLUDE_DIR}/core/io/certs_compressed.gen.h)
generate_file(certs_header ${GODOT_CORE_IO_CA_CERTIFICATES_SOURCE} ${GODOT_CORE_IO_CA_CERTIFICATES_DESTINATION})


add_custom_target(core-generated
        DEPENDS
        # object
        ${GODOT_CORE_OBJECT_DISABLED_CLASSES_OUTPUT}
        ${GODOT_INTERFACE_WRAPPERS_OUTPUT}
        ${GODOT_OBJECT_GD_VIRTUALS_OUTPUT}
        ${GODOT_CONFIG_VERSION_HEADER_OUTPUT}
        ${GODOT_CONFIG_VERSION_HASH_OUTPUT}
        ${GENERATED_MODULES_ENABLED_OUTPUT}
        ${GENERATED_MODULES_REGISTER_OUTPUT}

        # config
        ${GODOT_CORE_AUTHORS_SOURCE_OUTPUT}
        ${GODOT_CORE_DONORS_SOURCE_OUTPUT}
        ${GODOT_CORE_CONFIG_LICENSE_OUTPUT}
        ${GODOT_SCRIPT_ENCRYPTION_GENERATE_HEADER}

        # input
        ${GODOT_CORE_INPUT_CONTROLLER_MAPPINGS_OUTPUT}

        # io
        ${GODOT_CORE_IO_CA_CERTIFICATES_DESTINATION}
)
set_target_properties(core-generated PROPERTIES FOLDER "generated")
add_dependencies(core core-generated)

target_sources(core PRIVATE
        # object
        ${GODOT_CORE_OBJECT_DISABLED_CLASSES_OUTPUT}
        ${GODOT_INTERFACE_WRAPPERS_OUTPUT}
        ${GODOT_OBJECT_GD_VIRTUALS_OUTPUT}
        ${GODOT_CONFIG_VERSION_HEADER_OUTPUT}
        ${GODOT_CONFIG_VERSION_HASH_OUTPUT}
        ${GENERATED_MODULES_ENABLED_OUTPUT}
        ${GENERATED_MODULES_REGISTER_OUTPUT}

        # config
        ${GODOT_CORE_AUTHORS_SOURCE_OUTPUT}
        ${GODOT_CORE_DONORS_SOURCE_OUTPUT}
        ${GODOT_CORE_CONFIG_LICENSE_OUTPUT}
        ${GODOT_SCRIPT_ENCRYPTION_GENERATE_HEADER}

        # input
        ${GODOT_CORE_INPUT_CONTROLLER_MAPPINGS_OUTPUT}

        # io
        ${GODOT_CORE_IO_CA_CERTIFICATES_DESTINATION}
)

add_subdirectory(object)
add_subdirectory(math)
add_subdirectory(string)
add_subdirectory(config)
add_subdirectory(crypto)
add_subdirectory(debugger)
add_subdirectory(error)
add_subdirectory(extension)
add_subdirectory(input)
add_subdirectory(profiling)
add_subdirectory(templates)
add_subdirectory(io)
add_subdirectory(os)
add_subdirectory(variant)


# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(core PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(core PRIVATE
        core_bind.cpp
        core_bind.h
        core_constants.cpp
        core_constants.h
        core_globals.h
        core_string_names.h
        doc_data.cpp
        doc_data.h
        register_core_types.cpp
        register_core_types.h
        typedefs.h
        version.h
)
# [[[end]]]

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(core PUBLIC platform-linuxbsd)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(core PUBLIC platform-windows)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(core PUBLIC macos)
elseif (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    target_link_libraries(core PUBLIC ios)
else ()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif ()

group_target_sources(core)
