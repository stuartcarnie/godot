godot_add_library(ogg FOLDER thirdparty STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(ogg PRIVATE')
#   for file in list_files(cog, ['.', 'ogg']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(ogg PRIVATE
        bitwise.c
        crctable.h
        framing.c
        ogg/config_types.h
        ogg/ogg.h
        ogg/os_types.h
)
# [[[end]]]
target_include_directories(ogg PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
