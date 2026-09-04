# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        shader_baker_export_plugin.cpp
        shader_baker_export_plugin.h
)
# [[[end]]]

target_include_directories(editor PRIVATE .)
