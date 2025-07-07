# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        animation_bezier_editor.cpp
        animation_bezier_editor.h
        animation_blend_space_1d_editor.cpp
        animation_blend_space_1d_editor.h
        animation_blend_space_2d_editor.cpp
        animation_blend_space_2d_editor.h
        animation_blend_tree_editor_plugin.cpp
        animation_blend_tree_editor_plugin.h
        animation_library_editor.cpp
        animation_library_editor.h
        animation_player_editor_plugin.cpp
        animation_player_editor_plugin.h
        animation_state_machine_editor.cpp
        animation_state_machine_editor.h
        animation_track_editor.cpp
        animation_track_editor.h
        animation_track_editor_plugins.cpp
        animation_track_editor_plugins.h
        animation_tree_editor_plugin.cpp
        animation_tree_editor_plugin.h
)
# [[[end]]]
