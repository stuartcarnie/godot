# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('set_abs(RENDERER_SERVER_SHADERS ${CMAKE_CURRENT_SOURCE_DIR}')
#   for file in list_files(cog, 'renderer_rd/shaders', exts=['glsl'], recursive=True):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
set_abs(RENDERER_SERVER_SHADERS ${CMAKE_CURRENT_SOURCE_DIR}
        renderer_rd/shaders/blit.glsl
        renderer_rd/shaders/canvas.glsl
        renderer_rd/shaders/canvas_occlusion.glsl
        renderer_rd/shaders/canvas_sdf.glsl
        renderer_rd/shaders/canvas_uniforms_inc.glsl
        renderer_rd/shaders/cluster_data_inc.glsl
        renderer_rd/shaders/cluster_debug.glsl
        renderer_rd/shaders/cluster_render.glsl
        renderer_rd/shaders/cluster_store.glsl
        renderer_rd/shaders/decal_data_inc.glsl
        renderer_rd/shaders/effects/blur_raster.glsl
        renderer_rd/shaders/effects/blur_raster_inc.glsl
        renderer_rd/shaders/effects/bokeh_dof.glsl
        renderer_rd/shaders/effects/bokeh_dof_inc.glsl
        renderer_rd/shaders/effects/bokeh_dof_raster.glsl
        renderer_rd/shaders/effects/copy.glsl
        renderer_rd/shaders/effects/copy_to_fb.glsl
        renderer_rd/shaders/effects/cube_to_dp.glsl
        renderer_rd/shaders/effects/cubemap_downsampler.glsl
        renderer_rd/shaders/effects/cubemap_downsampler_inc.glsl
        renderer_rd/shaders/effects/cubemap_downsampler_raster.glsl
        renderer_rd/shaders/effects/cubemap_filter.glsl
        renderer_rd/shaders/effects/cubemap_filter_raster.glsl
        renderer_rd/shaders/effects/cubemap_roughness.glsl
        renderer_rd/shaders/effects/cubemap_roughness_inc.glsl
        renderer_rd/shaders/effects/cubemap_roughness_raster.glsl
        renderer_rd/shaders/effects/fsr2/fsr2_accumulate_pass.glsl
        renderer_rd/shaders/effects/fsr2/fsr2_autogen_reactive_pass.glsl
        renderer_rd/shaders/effects/fsr2/fsr2_compute_luminance_pyramid_pass.glsl
        renderer_rd/shaders/effects/fsr2/fsr2_depth_clip_pass.glsl
        renderer_rd/shaders/effects/fsr2/fsr2_lock_pass.glsl
        renderer_rd/shaders/effects/fsr2/fsr2_rcas_pass.glsl
        renderer_rd/shaders/effects/fsr2/fsr2_reconstruct_previous_depth_pass.glsl
        renderer_rd/shaders/effects/fsr2/fsr2_tcr_autogen_pass.glsl
        renderer_rd/shaders/effects/fsr_upscale.glsl
        renderer_rd/shaders/effects/luminance_reduce.glsl
        renderer_rd/shaders/effects/luminance_reduce_raster.glsl
        renderer_rd/shaders/effects/luminance_reduce_raster_inc.glsl
        renderer_rd/shaders/effects/motion_vector_inc.glsl
        renderer_rd/shaders/effects/motion_vectors.glsl
        renderer_rd/shaders/effects/resolve.glsl
        renderer_rd/shaders/effects/roughness_limiter.glsl
        renderer_rd/shaders/effects/screen_space_reflection.glsl
        renderer_rd/shaders/effects/screen_space_reflection_filter.glsl
        renderer_rd/shaders/effects/screen_space_reflection_inc.glsl
        renderer_rd/shaders/effects/screen_space_reflection_scale.glsl
        renderer_rd/shaders/effects/shadow_frustum.glsl
        renderer_rd/shaders/effects/sort.glsl
        renderer_rd/shaders/effects/specular_merge.glsl
        renderer_rd/shaders/effects/ss_effects_downsample.glsl
        renderer_rd/shaders/effects/ssao.glsl
        renderer_rd/shaders/effects/ssao_blur.glsl
        renderer_rd/shaders/effects/ssao_importance_map.glsl
        renderer_rd/shaders/effects/ssao_interleave.glsl
        renderer_rd/shaders/effects/ssil.glsl
        renderer_rd/shaders/effects/ssil_blur.glsl
        renderer_rd/shaders/effects/ssil_importance_map.glsl
        renderer_rd/shaders/effects/ssil_interleave.glsl
        renderer_rd/shaders/effects/subsurface_scattering.glsl
        renderer_rd/shaders/effects/taa_resolve.glsl
        renderer_rd/shaders/effects/tonemap.glsl
        renderer_rd/shaders/effects/vrs.glsl
        renderer_rd/shaders/environment/gi.glsl
        renderer_rd/shaders/environment/sdfgi_debug.glsl
        renderer_rd/shaders/environment/sdfgi_debug_probes.glsl
        renderer_rd/shaders/environment/sdfgi_direct_light.glsl
        renderer_rd/shaders/environment/sdfgi_integrate.glsl
        renderer_rd/shaders/environment/sdfgi_preprocess.glsl
        renderer_rd/shaders/environment/sky.glsl
        renderer_rd/shaders/environment/volumetric_fog.glsl
        renderer_rd/shaders/environment/volumetric_fog_process.glsl
        renderer_rd/shaders/environment/voxel_gi.glsl
        renderer_rd/shaders/environment/voxel_gi_debug.glsl
        renderer_rd/shaders/environment/voxel_gi_sdf.glsl
        renderer_rd/shaders/forward_clustered/best_fit_normal.glsl
        renderer_rd/shaders/forward_clustered/scene_forward_clustered.glsl
        renderer_rd/shaders/forward_clustered/scene_forward_clustered_inc.glsl
        renderer_rd/shaders/forward_mobile/scene_forward_mobile.glsl
        renderer_rd/shaders/forward_mobile/scene_forward_mobile_inc.glsl
        renderer_rd/shaders/giprobe_write.glsl
        renderer_rd/shaders/light_data_inc.glsl
        renderer_rd/shaders/particles.glsl
        renderer_rd/shaders/particles_copy.glsl
        renderer_rd/shaders/samplers_inc.glsl
        renderer_rd/shaders/scene_data_inc.glsl
        renderer_rd/shaders/scene_forward_aa_inc.glsl
        renderer_rd/shaders/scene_forward_gi_inc.glsl
        renderer_rd/shaders/scene_forward_lights_inc.glsl
        renderer_rd/shaders/skeleton.glsl
)
# [[[end]]]

set(RENDERER_SERVER_INCLUDE_DIR ${CMAKE_BINARY_INCLUDE_DIR}/servers/rendering)
string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR} ${RENDERER_SERVER_INCLUDE_DIR} RENDERER_SERVER_SHADERS_GENERATED "${RENDERER_SERVER_SHADERS}")
string(REPLACE "glsl" "glsl.gen.h" RENDERER_SERVER_SHADERS_GENERATED "${RENDERER_SERVER_SHADERS_GENERATED}")
generate_shader_sources(glsl "${RENDERER_SERVER_SHADERS}" "${RENDERER_SERVER_SHADERS_GENERATED}")

add_subdirectory(dummy)
add_subdirectory(renderer_rd)
add_subdirectory(storage)

godot_add_library(godot-server-renderer FOLDER servers STATIC ${RENDERER_SERVER_SHADERS_GENERATED})
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-server-renderer PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-server-renderer PRIVATE
        renderer_canvas_cull.cpp
        renderer_canvas_cull.h
        renderer_canvas_render.cpp
        renderer_canvas_render.h
        renderer_compositor.cpp
        renderer_compositor.h
        renderer_geometry_instance.cpp
        renderer_geometry_instance.h
        renderer_scene_cull.cpp
        renderer_scene_cull.h
        renderer_scene_occlusion_cull.cpp
        renderer_scene_occlusion_cull.h
        renderer_scene_render.cpp
        renderer_scene_render.h
        renderer_viewport.cpp
        renderer_viewport.h
        rendering_device.cpp
        rendering_device.h
        rendering_device_binds.cpp
        rendering_device_binds.h
        rendering_device_commons.cpp
        rendering_device_commons.h
        rendering_device_driver.cpp
        rendering_device_driver.h
        rendering_device_graph.cpp
        rendering_device_graph.h
        rendering_light_culler.cpp
        rendering_light_culler.h
        rendering_method.cpp
        rendering_method.h
        rendering_server_default.cpp
        rendering_server_default.h
        rendering_server_globals.cpp
        rendering_server_globals.h
        shader_compiler.cpp
        shader_compiler.h
        shader_language.cpp
        shader_language.h
        shader_preprocessor.cpp
        shader_preprocessor.h
        shader_types.cpp
        shader_types.h
        shader_warnings.cpp
        shader_warnings.h
)
# [[[end]]]
target_include_directories(godot-server-renderer PUBLIC ${CMAKE_BINARY_INCLUDE_DIR}) #Some other targets depend on specific generated shaders
target_link_libraries(godot-server-renderer PUBLIC godot-templates godot-os godot-glslang spirv-reflect)
