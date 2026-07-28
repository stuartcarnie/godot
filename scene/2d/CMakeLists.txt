add_subdirectory(navigation)
add_subdirectory(physics)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(scene PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(scene PRIVATE
        animated_sprite_2d.cpp
        animated_sprite_2d.h
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
        cpu_particles_2d.cpp
        cpu_particles_2d.h
        gpu_particles_2d.cpp
        gpu_particles_2d.h
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
        node_2d.cpp
        node_2d.h
        parallax_2d.cpp
        parallax_2d.h
        parallax_background.cpp
        parallax_background.h
        parallax_layer.cpp
        parallax_layer.h
        path_2d.cpp
        path_2d.h
        polygon_2d.cpp
        polygon_2d.h
        remote_transform_2d.cpp
        remote_transform_2d.h
        skeleton_2d.cpp
        skeleton_2d.h
        sprite_2d.cpp
        sprite_2d.h
        visible_on_screen_notifier_2d.cpp
        visible_on_screen_notifier_2d.h
)
# [[[end]]]
target_link_libraries(scene PUBLIC clipper2)

