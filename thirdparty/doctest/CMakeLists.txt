godot_add_library(doctest FOLDER thirdparty NO_INCLUDES INTERFACE)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(doctest PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(doctest PRIVATE
        doctest.h
)
# [[[end]]]

target_include_directories(doctest INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
