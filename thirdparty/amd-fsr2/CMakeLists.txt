godot_add_library(amd-fsr2 FOLDER thirdparty NO_INCLUDES STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(amd-fsr2 PRIVATE')
#   for file in list_files(cog, ['.', 'shaders']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(amd-fsr2 PRIVATE
        ffx_assert.cpp
        ffx_assert.h
        ffx_error.h
        ffx_fsr2.cpp
        ffx_fsr2.h
        ffx_fsr2_interface.h
        ffx_fsr2_maximum_bias.h
        ffx_fsr2_private.h
        ffx_types.h
        ffx_util.h
        shaders/ffx_common_types.h
        shaders/ffx_core.h
        shaders/ffx_core_cpu.h
        shaders/ffx_core_glsl.h
        shaders/ffx_core_gpu_common.h
        shaders/ffx_core_gpu_common_half.h
        shaders/ffx_core_hlsl.h
        shaders/ffx_core_portability.h
        shaders/ffx_fsr1.h
        shaders/ffx_fsr2_accumulate.h
        shaders/ffx_fsr2_callbacks_glsl.h
        shaders/ffx_fsr2_callbacks_hlsl.h
        shaders/ffx_fsr2_common.h
        shaders/ffx_fsr2_compute_luminance_pyramid.h
        shaders/ffx_fsr2_depth_clip.h
        shaders/ffx_fsr2_force16_begin.h
        shaders/ffx_fsr2_force16_end.h
        shaders/ffx_fsr2_lock.h
        shaders/ffx_fsr2_postprocess_lock_status.h
        shaders/ffx_fsr2_rcas.h
        shaders/ffx_fsr2_reconstruct_dilated_velocity_and_previous_depth.h
        shaders/ffx_fsr2_reproject.h
        shaders/ffx_fsr2_resources.h
        shaders/ffx_fsr2_sample.h
        shaders/ffx_fsr2_tcr_autogen.h
        shaders/ffx_fsr2_upsample.h
        shaders/ffx_spd.h
)
# [[[end]]]
target_include_directories(amd-fsr2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(amd-fsr2 PUBLIC FFX_GCC)
