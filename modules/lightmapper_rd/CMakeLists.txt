# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('set_abs(GODOT_MODULE_LIGHTMAPPER_RD_SHADERS ${CMAKE_CURRENT_SOURCE_DIR}')
#   for file in list_files(cog, '.', exts=['glsl']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
set_abs(GODOT_MODULE_LIGHTMAPPER_RD_SHADERS ${CMAKE_CURRENT_SOURCE_DIR}
        lm_area_lights_inc.glsl
        lm_blendseams.glsl
        lm_common_inc.glsl
        lm_compute.glsl
        lm_raster.glsl
)
# [[[end]]]

set(LIGHTMAPPER_GENERATED_OUTPUT_DIR ${CMAKE_BINARY_INCLUDE_DIR}/modules/lightmapper_rd)
string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR} ${LIGHTMAPPER_GENERATED_OUTPUT_DIR} GODOT_MODULE_LIGHTMAPPER_RD_SHADERS_GENERATED "${GODOT_MODULE_LIGHTMAPPER_RD_SHADERS}")
string(REPLACE "glsl" "glsl.gen.h" GODOT_MODULE_LIGHTMAPPER_RD_SHADERS_GENERATED "${GODOT_MODULE_LIGHTMAPPER_RD_SHADERS_GENERATED}")
generate_shader_sources(glsl_raw "${GODOT_MODULE_LIGHTMAPPER_RD_SHADERS}" "${GODOT_MODULE_LIGHTMAPPER_RD_SHADERS_GENERATED}")

add_custom_target(generate_lightmapper_rd_shaders DEPENDS ${GODOT_MODULE_LIGHTMAPPER_RD_SHADERS_GENERATED})
set_target_properties(generate_lightmapper_rd_shaders PROPERTIES FOLDER "generated")

godot_add_library(godot-module-lightmapper-rd FOLDER modules STATIC)
add_dependencies(godot-module-lightmapper-rd generate_lightmapper_rd_shaders)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-lightmapper-rd PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-lightmapper-rd PRIVATE
        lightmapper_rd.cpp
        lightmapper_rd.h
        register_types.cpp
        register_types.h
)
# [[[end]]]

target_link_libraries(godot-module-lightmapper-rd PUBLIC
        core
        $<$<BOOL:${WANT_VULKAN}>:vulkan>
        $<$<BOOL:${WANT_METAL}>:metal>
)
target_include_directories(godot-module-lightmapper-rd PRIVATE ${LIGHTMAPPER_GENERATED_OUTPUT_DIR})
target_compile_definitions(godot-module-lightmapper-rd
        PRIVATE
        $<$<AND:$<BOOL:${WANT_VULKAN}>,$<BOOL:${VULKAN_FOUND}>>:VULKAN_ENABLED RD_ENABLED>
        $<$<BOOL:${WANT_METAL}>:METAL_ENABLED RD_ENABLED>
)

group_target_sources(godot-module-lightmapper-rd)
