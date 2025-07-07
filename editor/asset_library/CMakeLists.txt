# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        asset_library_editor_plugin.cpp
        asset_library_editor_plugin.h
        editor_asset_installer.cpp
        editor_asset_installer.h
)
# [[[end]]]
