# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        physical_bone_3d_editor_plugin.cpp
        physical_bone_3d_editor_plugin.h
)
# [[[end]]]
