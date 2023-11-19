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

godot_add_library(godot-config FOLDER core STATIC)
target_sources(godot-config PRIVATE
        ${GODOT_CORE_AUTHORS_SOURCE_OUTPUT}
        ${GODOT_CORE_DONORS_SOURCE_OUTPUT}
        ${GODOT_CORE_CONFIG_LICENSE_OUTPUT}
        ${GODOT_SCRIPT_ENCRYPTION_GENERATE_HEADER}
)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-config PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-config PRIVATE
        engine.cpp
        engine.h
        project_settings.cpp
        project_settings.h
)
# [[[end]]]

target_include_directories(godot-config PUBLIC ${CMAKE_BINARY_INCLUDE_DIR})
target_link_libraries(godot-config PUBLIC godot-string godot-object)
if(${WANT_OPENGL} AND ${OPENGL_FOUND})
    target_compile_definitions(godot-config PUBLIC GLES3_ENABLED=1)
endif()
if(${WANT_VULKAN} AND ${VULKAN_FOUND})
    target_compile_definitions(godot-config PUBLIC VULKAN_ENABLED=1)
endif()

target_compile_definitions(godot-config
        PRIVATE
        $<$<BOOL:${METAL_ENABLED}>:METAL_ENABLED=1>
)
