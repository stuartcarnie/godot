godot_add_library(godot-editor-plugins-gizmos FOLDER plugins)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-editor-plugins-gizmos PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-editor-plugins-gizmos PRIVATE
        audio_listener_3d_gizmo_plugin.cpp
        audio_listener_3d_gizmo_plugin.h
        audio_stream_player_3d_gizmo_plugin.cpp
        audio_stream_player_3d_gizmo_plugin.h
        camera_3d_gizmo_plugin.cpp
        camera_3d_gizmo_plugin.h
        collision_object_3d_gizmo_plugin.cpp
        collision_object_3d_gizmo_plugin.h
        collision_polygon_3d_gizmo_plugin.cpp
        collision_polygon_3d_gizmo_plugin.h
        collision_shape_3d_gizmo_plugin.cpp
        collision_shape_3d_gizmo_plugin.h
        cpu_particles_3d_gizmo_plugin.cpp
        cpu_particles_3d_gizmo_plugin.h
        decal_gizmo_plugin.cpp
        decal_gizmo_plugin.h
        fog_volume_gizmo_plugin.cpp
        fog_volume_gizmo_plugin.h
        gizmo_3d_helper.cpp
        gizmo_3d_helper.h
        gpu_particles_3d_gizmo_plugin.cpp
        gpu_particles_3d_gizmo_plugin.h
        gpu_particles_collision_3d_gizmo_plugin.cpp
        gpu_particles_collision_3d_gizmo_plugin.h
        joint_3d_gizmo_plugin.cpp
        joint_3d_gizmo_plugin.h
        label_3d_gizmo_plugin.cpp
        label_3d_gizmo_plugin.h
        light_3d_gizmo_plugin.cpp
        light_3d_gizmo_plugin.h
        lightmap_gi_gizmo_plugin.cpp
        lightmap_gi_gizmo_plugin.h
        lightmap_probe_gizmo_plugin.cpp
        lightmap_probe_gizmo_plugin.h
        marker_3d_gizmo_plugin.cpp
        marker_3d_gizmo_plugin.h
        mesh_instance_3d_gizmo_plugin.cpp
        mesh_instance_3d_gizmo_plugin.h
        navigation_link_3d_gizmo_plugin.cpp
        navigation_link_3d_gizmo_plugin.h
        navigation_region_3d_gizmo_plugin.cpp
        navigation_region_3d_gizmo_plugin.h
        occluder_instance_3d_gizmo_plugin.cpp
        occluder_instance_3d_gizmo_plugin.h
        physics_bone_3d_gizmo_plugin.cpp
        physics_bone_3d_gizmo_plugin.h
        ray_cast_3d_gizmo_plugin.cpp
        ray_cast_3d_gizmo_plugin.h
        reflection_probe_gizmo_plugin.cpp
        reflection_probe_gizmo_plugin.h
        shape_cast_3d_gizmo_plugin.cpp
        shape_cast_3d_gizmo_plugin.h
        soft_body_3d_gizmo_plugin.cpp
        soft_body_3d_gizmo_plugin.h
        spring_arm_3d_gizmo_plugin.cpp
        spring_arm_3d_gizmo_plugin.h
        sprite_base_3d_gizmo_plugin.cpp
        sprite_base_3d_gizmo_plugin.h
        vehicle_body_3d_gizmo_plugin.cpp
        vehicle_body_3d_gizmo_plugin.h
        visible_on_screen_notifier_3d_gizmo_plugin.cpp
        visible_on_screen_notifier_3d_gizmo_plugin.h
        voxel_gi_gizmo_plugin.cpp
        voxel_gi_gizmo_plugin.h
)
# [[[end]]]
target_link_libraries(godot-editor-plugins-gizmos PRIVATE godot-config)
