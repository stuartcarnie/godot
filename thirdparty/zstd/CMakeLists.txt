godot_add_library(zstd FOLDER thirdparty STATIC)
target_sources(zstd PRIVATE
        common/debug.c
        common/entropy_common.c
        common/error_private.c
        common/fse_decompress.c
        common/pool.c
        common/threading.c
        common/xxhash.c
        common/zstd_common.c
        compress/fse_compress.c
        compress/hist.c
        compress/huf_compress.c
        compress/zstd_compress.c
        compress/zstd_compress_literals.c
        compress/zstd_compress_sequences.c
        compress/zstd_compress_superblock.c
        compress/zstd_double_fast.c
        compress/zstd_fast.c
        compress/zstd_lazy.c
        compress/zstd_ldm.c
        compress/zstd_opt.c
        compress/zstdmt_compress.c
        decompress/huf_decompress.c
        decompress/zstd_ddict.c
        decompress/zstd_decompress.c
        decompress/zstd_decompress_block.c
)
target_include_directories(zstd PUBLIC ${GODOT_SOURCE_ROOT_THIRDPARTY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(zstd PRIVATE ZSTD_DISABLE_ASM=1)
