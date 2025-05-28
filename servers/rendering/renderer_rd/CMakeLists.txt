# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('set_abs(RENDERER_RD_SERVER_SHADERS ${CMAKE_CURRENT_SOURCE_DIR}')
#   for file in list_files(cog, 'shaders', exts=['glsl'], gexclude='*_inc.glsl', recursive=True):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
set_abs(RENDERER_RD_SERVER_SHADERS ${CMAKE_CURRENT_SOURCE_DIR}
        shaders/blit.glsl
        shaders/canvas.glsl
        shaders/canvas_occlusion.glsl
        shaders/canvas_sdf.glsl
        shaders/cluster_debug.glsl
        shaders/cluster_render.glsl
        shaders/cluster_store.glsl
        shaders/effects/blur_raster.glsl
        shaders/effects/bokeh_dof.glsl
        shaders/effects/bokeh_dof_raster.glsl
        shaders/effects/copy.glsl
        shaders/effects/copy_to_fb.glsl
        shaders/effects/cube_to_dp.glsl
        shaders/effects/cubemap_downsampler.glsl
        shaders/effects/cubemap_downsampler_raster.glsl
        shaders/effects/cubemap_filter.glsl
        shaders/effects/cubemap_filter_raster.glsl
        shaders/effects/cubemap_roughness.glsl
        shaders/effects/cubemap_roughness_raster.glsl
        shaders/effects/fsr2/fsr2_accumulate_pass.glsl
        shaders/effects/fsr2/fsr2_autogen_reactive_pass.glsl
        shaders/effects/fsr2/fsr2_compute_luminance_pyramid_pass.glsl
        shaders/effects/fsr2/fsr2_depth_clip_pass.glsl
        shaders/effects/fsr2/fsr2_lock_pass.glsl
        shaders/effects/fsr2/fsr2_rcas_pass.glsl
        shaders/effects/fsr2/fsr2_reconstruct_previous_depth_pass.glsl
        shaders/effects/fsr2/fsr2_tcr_autogen_pass.glsl
        shaders/effects/fsr_upscale.glsl
        shaders/effects/luminance_reduce.glsl
        shaders/effects/luminance_reduce_raster.glsl
        shaders/effects/motion_vectors.glsl
        shaders/effects/motion_vectors_store.glsl
        shaders/effects/resolve.glsl
        shaders/effects/roughness_limiter.glsl
        shaders/effects/screen_space_reflection.glsl
        shaders/effects/screen_space_reflection_filter.glsl
        shaders/effects/screen_space_reflection_scale.glsl
        shaders/effects/shadow_frustum.glsl
        shaders/effects/smaa_blending.glsl
        shaders/effects/smaa_edge_detection.glsl
        shaders/effects/smaa_weight_calculation.glsl
        shaders/effects/sort.glsl
        shaders/effects/specular_merge.glsl
        shaders/effects/ss_effects_downsample.glsl
        shaders/effects/ssao.glsl
        shaders/effects/ssao_blur.glsl
        shaders/effects/ssao_importance_map.glsl
        shaders/effects/ssao_interleave.glsl
        shaders/effects/ssil.glsl
        shaders/effects/ssil_blur.glsl
        shaders/effects/ssil_importance_map.glsl
        shaders/effects/ssil_interleave.glsl
        shaders/effects/subsurface_scattering.glsl
        shaders/effects/taa_resolve.glsl
        shaders/effects/tonemap.glsl
        shaders/effects/vrs.glsl
        shaders/environment/gi.glsl
        shaders/environment/sdfgi_debug.glsl
        shaders/environment/sdfgi_debug_probes.glsl
        shaders/environment/sdfgi_direct_light.glsl
        shaders/environment/sdfgi_integrate.glsl
        shaders/environment/sdfgi_preprocess.glsl
        shaders/environment/sky.glsl
        shaders/environment/volumetric_fog.glsl
        shaders/environment/volumetric_fog_process.glsl
        shaders/environment/voxel_gi.glsl
        shaders/environment/voxel_gi_debug.glsl
        shaders/environment/voxel_gi_sdf.glsl
        shaders/forward_clustered/best_fit_normal.glsl
        shaders/forward_clustered/integrate_dfg.glsl
        shaders/forward_clustered/scene_forward_clustered.glsl
        shaders/forward_mobile/scene_forward_mobile.glsl
        shaders/giprobe_write.glsl
        shaders/particles.glsl
        shaders/particles_copy.glsl
        shaders/skeleton.glsl
)
# [[[end]]]

set(RENDERER_RD_SERVER_INCLUDE_DIR ${CMAKE_BINARY_INCLUDE_DIR}/servers/rendering/renderer_rd)
string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR} ${RENDERER_RD_SERVER_INCLUDE_DIR} RENDERER_RD_SERVER_SHADERS_GENERATED "${RENDERER_RD_SERVER_SHADERS}")
string(REPLACE "glsl" "glsl.gen.h" RENDERER_RD_SERVER_SHADERS_GENERATED "${RENDERER_RD_SERVER_SHADERS_GENERATED}")
generate_shader_sources(glsl "${RENDERER_RD_SERVER_SHADERS}" "${RENDERER_RD_SERVER_SHADERS_GENERATED}")

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('set_abs(RENDERER_RD_SERVER_SHADERS_INC ${CMAKE_CURRENT_SOURCE_DIR}')
#   for file in list_files(cog, 'shaders', exts=['glsl'], ginclude='*_inc.glsl'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
set_abs(RENDERER_RD_SERVER_SHADERS_INC ${CMAKE_CURRENT_SOURCE_DIR}
        shaders/canvas_uniforms_inc.glsl
        shaders/cluster_data_inc.glsl
        shaders/decal_data_inc.glsl
        shaders/light_data_inc.glsl
        shaders/samplers_inc.glsl
        shaders/scene_data_inc.glsl
        shaders/scene_forward_aa_inc.glsl
        shaders/scene_forward_gi_inc.glsl
        shaders/scene_forward_lights_inc.glsl
        shaders/scene_forward_vertex_lights_inc.glsl
)
# [[[end]]]

string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR} ${RENDERER_RD_SERVER_INCLUDE_DIR} RENDERER_RD_SERVER_SHADERS_INC_GENERATED "${RENDERER_RD_SERVER_SHADERS_INC}")
string(REPLACE "glsl" "glsl.gen.h" RENDERER_RD_SERVER_SHADERS_INC_GENERATED "${RENDERER_RD_SERVER_SHADERS_INC_GENERATED}")
generate_shader_sources(glsl_raw "${RENDERER_RD_SERVER_SHADERS_INC}" "${RENDERER_RD_SERVER_SHADERS_INC_GENERATED}")

add_custom_target(servers-renderer_rd-generated
        DEPENDS
        ${RENDERER_RD_SERVER_SHADERS_GENERATED}
        ${RENDERER_RD_SERVER_SHADERS_INC_GENERATED}
)
set_target_properties(servers-renderer_rd-generated PROPERTIES FOLDER "generated")

target_sources(servers
        PRIVATE
        ${RENDERER_RD_SERVER_SHADERS_GENERATED}
        ${RENDERER_RD_SERVER_SHADERS_INC_GENERATED}
)

add_dependencies(servers servers-renderer_rd-generated)

add_subdirectory(effects)
add_subdirectory(environment)
add_subdirectory(forward_clustered)
add_subdirectory(forward_mobile)
add_subdirectory(storage_rd)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(servers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(servers PRIVATE
        cluster_builder_rd.cpp
        cluster_builder_rd.h
        framebuffer_cache_rd.cpp
        framebuffer_cache_rd.h
        pipeline_cache_rd.cpp
        pipeline_cache_rd.h
        pipeline_hash_map_rd.h
        renderer_canvas_render_rd.cpp
        renderer_canvas_render_rd.h
        renderer_compositor_rd.cpp
        renderer_compositor_rd.h
        renderer_scene_render_rd.cpp
        renderer_scene_render_rd.h
        shader_rd.cpp
        shader_rd.h
        uniform_set_cache_rd.cpp
        uniform_set_cache_rd.h
)
# [[[end]]]
target_link_libraries(servers PUBLIC godot-glslang spirv-reflect smaa)

target_compile_definitions(servers
        PRIVATE
        $<$<BOOL:${WANT_DYNAMIC_CORE_SHADERS}>:DYNAMIC_CORE_SHADERS GODOT_SOURCE_ROOT="${CMAKE_SOURCE_DIR}">
)
