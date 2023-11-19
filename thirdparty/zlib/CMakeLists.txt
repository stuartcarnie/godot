godot_add_library(zlib FOLDER thirdparty STATIC)
target_sources(zlib PRIVATE
        adler32.c
        compress.c
        crc32.c
        deflate.c
        inffast.c
        inflate.c
        inftrees.c
        trees.c
        uncompr.c
        zutil.c
)
target_include_directories(zlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(zlib PRIVATE $<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>)
