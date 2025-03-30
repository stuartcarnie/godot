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

target_sources(core PRIVATE
        ${GODOT_CORE_AUTHORS_SOURCE_OUTPUT}
        ${GODOT_CORE_DONORS_SOURCE_OUTPUT}
        ${GODOT_CORE_CONFIG_LICENSE_OUTPUT}
        ${GODOT_SCRIPT_ENCRYPTION_GENERATE_HEADER}
)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(core PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(core PRIVATE
        engine.cpp
        engine.h
        project_settings.cpp
        project_settings.h
)
# [[[end]]]

target_include_directories(core PUBLIC ${CMAKE_BINARY_INCLUDE_DIR})

target_compile_definitions(core
        PRIVATE
        $<$<AND:$<BOOL:${WANT_OPENGL}>,$<BOOL:OPENGL_FOUND}>>:GLES3_ENABLED=1>
        $<$<AND:$<BOOL:${WANT_VULKAN}>,$<BOOL:${VULKAN_FOUND}>>:VULKAN_ENABLED=1 RD_ENABLED=1>
        $<$<BOOL:${WANT_METAL}>:METAL_ENABLED=1 RD_ENABLED=1>
)
