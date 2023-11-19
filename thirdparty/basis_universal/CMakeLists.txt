godot_add_library(basis_universal FOLDER thirdparty STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(basis_universal PRIVATE')
#   for file in list_files(cog, ['encoder', 'transcoder']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(basis_universal PRIVATE
        encoder/basisu_backend.cpp
        encoder/basisu_backend.h
        encoder/basisu_basis_file.cpp
        encoder/basisu_basis_file.h
        encoder/basisu_bc7enc.cpp
        encoder/basisu_bc7enc.h
        encoder/basisu_comp.cpp
        encoder/basisu_comp.h
        encoder/basisu_enc.cpp
        encoder/basisu_enc.h
        encoder/basisu_etc.cpp
        encoder/basisu_etc.h
        encoder/basisu_frontend.cpp
        encoder/basisu_frontend.h
        encoder/basisu_gpu_texture.cpp
        encoder/basisu_gpu_texture.h
        encoder/basisu_kernels_declares.h
        encoder/basisu_kernels_imp.h
        encoder/basisu_kernels_sse.cpp
        encoder/basisu_miniz.h
        encoder/basisu_ocl_kernels.h
        encoder/basisu_opencl.cpp
        encoder/basisu_opencl.h
        encoder/basisu_pvrtc1_4.cpp
        encoder/basisu_pvrtc1_4.h
        encoder/basisu_resample_filters.cpp
        encoder/basisu_resampler.cpp
        encoder/basisu_resampler.h
        encoder/basisu_resampler_filters.h
        encoder/basisu_ssim.cpp
        encoder/basisu_ssim.h
        encoder/basisu_uastc_enc.cpp
        encoder/basisu_uastc_enc.h
        encoder/cppspmd_flow.h
        encoder/cppspmd_math.h
        encoder/cppspmd_math_declares.h
        encoder/cppspmd_sse.h
        encoder/cppspmd_type_aliases.h
        encoder/jpgd.cpp
        encoder/jpgd.h
        encoder/pvpngreader.cpp
        encoder/pvpngreader.h
        transcoder/basisu.h
        transcoder/basisu_containers.h
        transcoder/basisu_containers_impl.h
        transcoder/basisu_file_headers.h
        transcoder/basisu_transcoder.cpp
        transcoder/basisu_transcoder.h
        transcoder/basisu_transcoder_internal.h
        transcoder/basisu_transcoder_uastc.h
)
# [[[end]]]
target_link_libraries(basis_universal PUBLIC zstd)
target_include_directories(basis_universal PRIVATE .)
target_include_directories(basis_universal PUBLIC .)
target_compile_options(basis_universal PUBLIC $<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>)