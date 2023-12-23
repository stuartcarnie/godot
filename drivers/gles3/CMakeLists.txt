# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('set_abs(GLES3_GLSL_SHADERS ${CMAKE_CURRENT_SOURCE_DIR}')
#   for file in list_files(cog, 'shaders', exts=['glsl']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
set_abs(GLES3_GLSL_SHADERS ${CMAKE_CURRENT_SOURCE_DIR}
        shaders/canvas.glsl
        shaders/canvas_occlusion.glsl
        shaders/canvas_sdf.glsl
        shaders/canvas_uniforms_inc.glsl
        shaders/copy.glsl
        shaders/cube_to_dp.glsl
        shaders/cubemap_filter.glsl
        shaders/effect_blur.glsl
        shaders/lens_distorted.glsl
        shaders/particles.glsl
        shaders/particles_copy.glsl
        shaders/scene.glsl
        shaders/skeleton.glsl
        shaders/sky.glsl
        shaders/stdlib_inc.glsl
        shaders/tonemap.glsl
        shaders/tonemap_inc.glsl
)
# [[[end]]]

#Generate output filenames for GLES3 shaders to the c++ glsl.gen
set(GLES3_DRIVER_SHADER_INCLUDE_DIR ${CMAKE_BINARY_INCLUDE_DIR}/drivers/gles3)
string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR} ${GLES3_DRIVER_SHADER_INCLUDE_DIR} GLES3_GLSL_SHADERS_GENERATED "${GLES3_GLSL_SHADERS}")
string(REPLACE "glsl" "glsl.gen.h" GLES3_GLSL_SHADERS_GENERATED "${GLES3_GLSL_SHADERS_GENERATED}")
generate_shader_sources(gles3 "${GLES3_GLSL_SHADERS}" "${GLES3_GLSL_SHADERS_GENERATED}")

godot_add_library(driver-gles3 FOLDER drivers STATIC ${GLES3_GLSL_SHADERS_GENERATED})
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(driver-gles3 PRIVATE')
#   for file in list_files(cog, ['.', 'effects', 'environment', 'storage']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(driver-gles3 PRIVATE
        effects/copy_effects.cpp
        effects/copy_effects.h
        environment/fog.cpp
        environment/fog.h
        environment/gi.cpp
        environment/gi.h
        rasterizer_canvas_gles3.cpp
        rasterizer_canvas_gles3.h
        rasterizer_gles3.cpp
        rasterizer_gles3.h
        rasterizer_scene_gles3.cpp
        rasterizer_scene_gles3.h
        shader_gles3.cpp
        shader_gles3.h
        storage/config.cpp
        storage/config.h
        storage/light_storage.cpp
        storage/light_storage.h
        storage/material_storage.cpp
        storage/material_storage.h
        storage/mesh_storage.cpp
        storage/mesh_storage.h
        storage/particles_storage.cpp
        storage/particles_storage.h
        storage/render_scene_buffers_gles3.cpp
        storage/render_scene_buffers_gles3.h
        storage/texture_storage.cpp
        storage/texture_storage.h
        storage/utilities.cpp
        storage/utilities.h
)
# [[[end]]]

target_include_directories(driver-gles3 PUBLIC ${CMAKE_BINARY_INCLUDE_DIR})
target_link_libraries(driver-gles3 PUBLIC godot-core $<${WANT_GLAD}:glad>)
target_compile_definitions(driver-gles3 PUBLIC
        GLES3_ENABLED=1
        $<${WANT_GLAD}:EGL_ENABLED=1 GLAD_ENABLED=1>
)