godot_add_library(jpeg-compressor FOLDER thirdparty STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(jpeg-compressor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(jpeg-compressor PRIVATE
        jpgd.cpp
        jpgd.h
        jpgd_idct.h
        jpge.cpp
        jpge.h
)
# [[[end]]]
target_include_directories(jpeg-compressor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
