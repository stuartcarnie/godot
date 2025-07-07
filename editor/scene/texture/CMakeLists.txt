# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        bit_map_editor_plugin.cpp
        bit_map_editor_plugin.h
        color_channel_selector.cpp
        color_channel_selector.h
        gradient_texture_2d_editor_plugin.cpp
        gradient_texture_2d_editor_plugin.h
        texture_3d_editor_plugin.cpp
        texture_3d_editor_plugin.h
        texture_editor_plugin.cpp
        texture_editor_plugin.h
        texture_layered_editor_plugin.cpp
        texture_layered_editor_plugin.h
        texture_region_editor_plugin.cpp
        texture_region_editor_plugin.h
)
# [[[end]]]
