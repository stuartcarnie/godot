godot_add_library(godot-scene-2d FOLDER scene STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-scene-2d PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-scene-2d PRIVATE
        animated_sprite_2d.cpp
        animated_sprite_2d.h
        area_2d.cpp
        area_2d.h
        audio_listener_2d.cpp
        audio_listener_2d.h
        audio_stream_player_2d.cpp
        audio_stream_player_2d.h
        back_buffer_copy.cpp
        back_buffer_copy.h
        camera_2d.cpp
        camera_2d.h
        canvas_group.cpp
        canvas_group.h
        canvas_modulate.cpp
        canvas_modulate.h
        collision_object_2d.cpp
        collision_object_2d.h
        collision_polygon_2d.cpp
        collision_polygon_2d.h
        collision_shape_2d.cpp
        collision_shape_2d.h
        cpu_particles_2d.cpp
        cpu_particles_2d.h
        gpu_particles_2d.cpp
        gpu_particles_2d.h
        joint_2d.cpp
        joint_2d.h
        light_2d.cpp
        light_2d.h
        light_occluder_2d.cpp
        light_occluder_2d.h
        line_2d.cpp
        line_2d.h
        line_builder.cpp
        line_builder.h
        marker_2d.cpp
        marker_2d.h
        mesh_instance_2d.cpp
        mesh_instance_2d.h
        multimesh_instance_2d.cpp
        multimesh_instance_2d.h
        navigation_agent_2d.cpp
        navigation_agent_2d.h
        navigation_link_2d.cpp
        navigation_link_2d.h
        navigation_obstacle_2d.cpp
        navigation_obstacle_2d.h
        navigation_region_2d.cpp
        navigation_region_2d.h
        node_2d.cpp
        node_2d.h
        parallax_background.cpp
        parallax_background.h
        parallax_layer.cpp
        parallax_layer.h
        path_2d.cpp
        path_2d.h
        physical_bone_2d.cpp
        physical_bone_2d.h
        physics_body_2d.cpp
        physics_body_2d.h
        polygon_2d.cpp
        polygon_2d.h
        ray_cast_2d.cpp
        ray_cast_2d.h
        remote_transform_2d.cpp
        remote_transform_2d.h
        shape_cast_2d.cpp
        shape_cast_2d.h
        skeleton_2d.cpp
        skeleton_2d.h
        sprite_2d.cpp
        sprite_2d.h
        tile_map.cpp
        tile_map.h
        touch_screen_button.cpp
        touch_screen_button.h
        visible_on_screen_notifier_2d.cpp
        visible_on_screen_notifier_2d.h
)
# [[[end]]]
target_link_libraries(godot-scene-2d PUBLIC godot-templates)

