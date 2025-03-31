# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(scene PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(scene PRIVATE
        animation_blend_space_1d.cpp
        animation_blend_space_1d.h
        animation_blend_space_2d.cpp
        animation_blend_space_2d.h
        animation_blend_tree.cpp
        animation_blend_tree.h
        animation_mixer.cpp
        animation_mixer.h
        animation_node_extension.cpp
        animation_node_extension.h
        animation_node_state_machine.cpp
        animation_node_state_machine.h
        animation_player.cpp
        animation_player.h
        animation_tree.cpp
        animation_tree.h
        easing_equations.h
        root_motion_view.cpp
        root_motion_view.h
        tween.cpp
        tween.h
)
# [[[end]]]
