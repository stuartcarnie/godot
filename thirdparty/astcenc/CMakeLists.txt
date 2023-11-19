godot_add_library(astcenc FOLDER thirdparty STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(astcenc PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(astcenc PRIVATE
        astcenc.h
        astcenc_averages_and_directions.cpp
        astcenc_block_sizes.cpp
        astcenc_color_quantize.cpp
        astcenc_color_unquantize.cpp
        astcenc_compress_symbolic.cpp
        astcenc_compute_variance.cpp
        astcenc_decompress_symbolic.cpp
        astcenc_diagnostic_trace.cpp
        astcenc_diagnostic_trace.h
        astcenc_entry.cpp
        astcenc_find_best_partitioning.cpp
        astcenc_ideal_endpoints_and_weights.cpp
        astcenc_image.cpp
        astcenc_integer_sequence.cpp
        astcenc_internal.h
        astcenc_internal_entry.h
        astcenc_mathlib.cpp
        astcenc_mathlib.h
        astcenc_mathlib_softfloat.cpp
        astcenc_partition_tables.cpp
        astcenc_percentile_tables.cpp
        astcenc_pick_best_endpoint_format.cpp
        astcenc_quantization.cpp
        astcenc_symbolic_physical.cpp
        astcenc_vecmathlib.h
        astcenc_vecmathlib_avx2_8.h
        astcenc_vecmathlib_common_4.h
        astcenc_vecmathlib_neon_4.h
        astcenc_vecmathlib_none_4.h
        astcenc_vecmathlib_sse_4.h
        astcenc_weight_align.cpp
        astcenc_weight_quant_xfer_tables.cpp
)
# [[[end]]]
target_include_directories(astcenc PUBLIC .)
