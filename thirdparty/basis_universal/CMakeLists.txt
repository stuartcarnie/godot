godot_add_library(basis_universal FOLDER thirdparty STATIC)
if (WANT_EDITOR)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('    target_sources(basis_universal PRIVATE')
    #   for file in list_files(cog, ['encoder', 'encoder/3rdparty']):
    #       cog.outl(f'            {file}')
    #   cog.outl('    )')
    # ]]]
    target_sources(basis_universal PRIVATE
            encoder/3rdparty/android_astc_decomp.cpp
            encoder/3rdparty/android_astc_decomp.h
            encoder/basisu_astc_hdr_6x6_enc.cpp
            encoder/basisu_astc_hdr_6x6_enc.h
            encoder/basisu_astc_hdr_common.cpp
            encoder/basisu_astc_hdr_common.h
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
            encoder/basisu_math.h
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
            encoder/basisu_uastc_hdr_4x4_enc.cpp
            encoder/basisu_uastc_hdr_4x4_enc.h
            encoder/cppspmd_flow.h
            encoder/cppspmd_math.h
            encoder/cppspmd_math_declares.h
            encoder/cppspmd_sse.h
            encoder/cppspmd_type_aliases.h
            encoder/jpgd.cpp
            encoder/jpgd.h
            encoder/pvpngreader.cpp
            encoder/pvpngreader.h
    )
    # [[[end]]]
endif ()

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(basis_universal PRIVATE')
#   for file in list_files(cog, 'transcoder'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(basis_universal PRIVATE
        transcoder/basisu.h
        transcoder/basisu_astc_hdr_core.h
        transcoder/basisu_astc_helpers.h
        transcoder/basisu_containers.h
        transcoder/basisu_containers_impl.h
        transcoder/basisu_file_headers.h
        transcoder/basisu_transcoder.cpp
        transcoder/basisu_transcoder.h
        transcoder/basisu_transcoder_internal.h
        transcoder/basisu_transcoder_uastc.h
)
# [[[end]]]
target_link_libraries(basis_universal PRIVATE zstd jpeg-turbo ktx tinyexr)
target_include_directories(basis_universal PUBLIC .)
target_compile_options(basis_universal PUBLIC $<$<COMPILE_LANGUAGE:CXX,C,OBJC,OBJCXX>:$<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>>)
target_compile_definitions(basis_universal
        PRIVATE
        # Storage formats.
        # Godot only implements `.basis` support through basis_universal.
        # Support for `.ktx` files are implemented with a direct libktx implementation.
        # Building the encoder requires `BASISD_SUPPORT_KTX2` to be enabled,
        # so we can only disable Zstandard compression for `.ktx` files
        # (this is not used in `.basis` files).
        BASISD_SUPPORT_KTX2_ZSTD=0
        # GPU compression formats.
        BASISD_SUPPORT_ATC=0        # Proprietary Adreno format not supported by Godot.
        BASISD_SUPPORT_FXT1=0       # Legacy format not supported by Godot.
        BASISD_SUPPORT_PVRTC1=0     # Legacy format not supported by Godot.
        BASISD_SUPPORT_PVRTC2=0     # Legacy format not supported by Godot.
        $<$<BOOL:${WANT_EDITOR}>:MINIZ_HEADER_FILE_ONLY>)
