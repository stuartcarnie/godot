godot_add_library(minizip FOLDER thirdparty STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(minizip PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(minizip PRIVATE
        crypt.h
        ioapi.c
        ioapi.h
        unzip.c
        unzip.h
        zip.c
        zip.h
)
# [[[end]]]
target_include_directories(minizip PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(minizip PUBLIC zlib)
