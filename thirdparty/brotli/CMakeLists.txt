godot_add_library(brotli FOLDER thirdparty STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(brotli PRIVATE')
#   for file in list_files(cog, ['common', 'dec', 'include']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(brotli PRIVATE
        common/constants.c
        common/constants.h
        common/context.c
        common/context.h
        common/dictionary.c
        common/dictionary.h
        common/platform.c
        common/platform.h
        common/shared_dictionary.c
        common/shared_dictionary_internal.h
        common/transform.c
        common/transform.h
        common/version.h
        dec/bit_reader.c
        dec/bit_reader.h
        dec/decode.c
        dec/huffman.c
        dec/huffman.h
        dec/prefix.h
        dec/state.c
        dec/state.h
)
# [[[end]]]
target_include_directories(brotli PUBLIC include)
