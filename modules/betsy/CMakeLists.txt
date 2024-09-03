# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('set_abs(GODOT_MODULE_BETSY_SHADERS ${CMAKE_CURRENT_SOURCE_DIR}')
#   for file in list_files(cog, '.', exts=['glsl']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
set_abs(GODOT_MODULE_BETSY_SHADERS ${CMAKE_CURRENT_SOURCE_DIR}
        CrossPlatformSettings_piece_all.glsl
        UavCrossPlatform_piece_all.glsl
        bc6h.glsl
)
# [[[end]]]

set(BETSY_GENERATED_OUTPUT_DIR ${CMAKE_BINARY_INCLUDE_DIR}/modules/betsy)
string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR} ${BETSY_GENERATED_OUTPUT_DIR} GODOT_MODULE_BETSY_SHADERS_GENERATED "${GODOT_MODULE_BETSY_SHADERS}")
string(REPLACE "glsl" "glsl.gen.h" GODOT_MODULE_BETSY_SHADERS_GENERATED "${GODOT_MODULE_BETSY_SHADERS_GENERATED}")
generate_shader_sources(glsl_raw "${GODOT_MODULE_BETSY_SHADERS}" "${GODOT_MODULE_BETSY_SHADERS_GENERATED}")

#Apply them to the library
godot_add_library(godot-module-betsy FOLDER modules STATIC ${GODOT_MODULE_BETSY_SHADERS_GENERATED})
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-betsy PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-betsy PRIVATE
        image_compress_betsy.cpp
        image_compress_betsy.h
        register_types.cpp
        register_types.h
)
# [[[end]]]

target_link_libraries(godot-module-betsy PUBLIC
        godot-object
        $<$<BOOL:${WANT_VULKAN}>:vulkan>
        $<$<BOOL:${WANT_METAL}>:metal>
)
target_include_directories(godot-module-betsy PRIVATE ${BETSY_GENERATED_OUTPUT_DIR})
target_compile_definitions(godot-module-betsy
        PRIVATE
        $<$<AND:$<BOOL:${WANT_VULKAN}>,$<BOOL:${VULKAN_FOUND}>>:VULKAN_ENABLED=1 RD_ENABLED=1>
        $<$<BOOL:${WANT_METAL}>:METAL_ENABLED=1 RD_ENABLED=1>
)
