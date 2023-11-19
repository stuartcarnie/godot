godot_add_library(godot-scene-animation FOLDER scene STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-scene-animation PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-scene-animation PRIVATE
        animation_blend_space_1d.cpp
        animation_blend_space_1d.h
        animation_blend_space_2d.cpp
        animation_blend_space_2d.h
        animation_blend_tree.cpp
        animation_blend_tree.h
        animation_mixer.cpp
        animation_mixer.h
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
target_link_libraries(godot-scene-animation PUBLIC godot-math godot-scene-2d godot-scene-3d)
set_property(TARGET godot-scene-animation PROPERTY CXX_STANDARD 17)
