godot_add_library(minimp3 FOLDER NO_INCLUDES thirdparty INTERFACE)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(minimp3 PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(minimp3 PRIVATE
        minimp3.h
        minimp3_ex.h
)
# [[[end]]]
target_include_directories(minimp3 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
