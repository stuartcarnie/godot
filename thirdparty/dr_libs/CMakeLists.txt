godot_add_library(dr_libs FOLDER thirdparty NO_INCLUDES INTERFACE)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(dr_libs PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(dr_libs PRIVATE
        dr_bridge.h
        dr_mp3.h
)
# [[[end]]]

target_include_directories(dr_libs INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
