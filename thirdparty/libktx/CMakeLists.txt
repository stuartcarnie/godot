godot_add_library(ktx FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(ktx PRIVATE')
#   for file in list_files(cog, ['lib', 'external/dfdutils']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(ktx PRIVATE
        external/dfdutils/colourspaces.c
        external/dfdutils/createdfd.c
        external/dfdutils/dfd.h
        external/dfdutils/interpretdfd.c
        external/dfdutils/printdfd.c
        external/dfdutils/queries.c
        external/dfdutils/vk2dfd.c
        lib/basis_sgd.h
        lib/basis_transcode.cpp
        lib/checkheader.c
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
        lib/vkformat_check_variant.c
        lib/vkformat_enum.h
        lib/vkformat_typesize.c
)
# [[[end]]]
target_compile_definitions(ktx PRIVATE KHRONOS_STATIC=1)
target_include_directories(ktx PUBLIC
        include
        utils
        lib
        other_include
        external
)

if (WANT_VULKAN)
    target_link_libraries(ktx PRIVATE vulkan)
else ()
    # Falls back on bundled `vkformat_enum.h`.
    target_compile_definitions(ktx PRIVATE LIBKTX)
endif ()

target_link_libraries(ktx PRIVATE basis_universal zstd)
target_compile_options(ktx PRIVATE $<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>) # disable all warnings
