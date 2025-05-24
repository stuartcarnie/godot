godot_add_library(grisu2 FOLDER thirdparty NO_INCLUDES INTERFACE)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(grisu2 PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(grisu2 PRIVATE
        grisu2.h
)
# [[[end]]]

target_include_directories(grisu2 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
