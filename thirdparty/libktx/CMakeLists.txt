godot_add_library(ktx FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(ktx PRIVATE')
#   for file in list_files(cog, ['lib', 'lib/dfdutils']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(ktx PRIVATE
        lib/basis_sgd.h
        lib/basis_transcode.cpp
        lib/checkheader.c
        lib/dfdutils/colourspaces.c
        lib/dfdutils/createdfd.c
        lib/dfdutils/dfd.h
        lib/dfdutils/interpretdfd.c
        lib/dfdutils/printdfd.c
        lib/dfdutils/queries.c
        lib/dfdutils/vk2dfd.c
        lib/filestream.c
        lib/filestream.h
        lib/formatsize.h
        lib/gl_format.h
        lib/hashlist.c
        lib/ktxint.h
        lib/memstream.c
        lib/memstream.h
        lib/miniz_wrapper.cpp
        lib/swap.c
        lib/texture.c
        lib/texture.h
        lib/texture1.c
        lib/texture1.h
        lib/texture2.c
        lib/texture2.h
        lib/uthash.h
        lib/vk_format.h
        lib/vkformat_check.c
        lib/vkformat_enum.h
)
# [[[end]]]
target_compile_definitions(ktx PRIVATE KHRONOS_STATIC=1)
target_include_directories(ktx PUBLIC
        include
        utils
        lib
        other_include
)

if (WANT_VULKAN)
    target_link_libraries(ktx PRIVATE vulkan)
else ()
    # Falls back on bundled `vkformat_enum.h`.
    target_compile_definitions(ktx PRIVATE LIBKTX)
endif ()

target_link_libraries(ktx PRIVATE basis_universal zstd)
target_compile_options(ktx PRIVATE $<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>) # disable all warnings
