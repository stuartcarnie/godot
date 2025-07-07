# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        editor_vcs_interface.cpp
        editor_vcs_interface.h
        version_control_editor_plugin.cpp
        version_control_editor_plugin.h
)
# [[[end]]]
