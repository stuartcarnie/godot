add_subdirectory(physics)

godot_add_library(godot-scene-3d FOLDER scene STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-scene-3d PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-scene-3d PRIVATE
        audio_listener_3d.cpp
        audio_listener_3d.h
        audio_stream_player_3d.cpp
        audio_stream_player_3d.h
        bone_attachment_3d.cpp
        bone_attachment_3d.h
        camera_3d.cpp
        camera_3d.h
        cpu_particles_3d.cpp
        cpu_particles_3d.h
        decal.cpp
        decal.h
        fog_volume.cpp
        fog_volume.h
        gpu_particles_3d.cpp
        gpu_particles_3d.h
        gpu_particles_collision_3d.cpp
        gpu_particles_collision_3d.h
        importer_mesh_instance_3d.cpp
        importer_mesh_instance_3d.h
        label_3d.cpp
        label_3d.h
        light_3d.cpp
        light_3d.h
        lightmap_gi.cpp
        lightmap_gi.h
        lightmap_probe.cpp
        lightmap_probe.h
        lightmapper.cpp
        lightmapper.h
        marker_3d.cpp
        marker_3d.h
        mesh_instance_3d.cpp
        mesh_instance_3d.h
        multimesh_instance_3d.cpp
        multimesh_instance_3d.h
        navigation_agent_3d.cpp
        navigation_agent_3d.h
        navigation_link_3d.cpp
        navigation_link_3d.h
        navigation_obstacle_3d.cpp
        navigation_obstacle_3d.h
        navigation_region_3d.cpp
        navigation_region_3d.h
        node_3d.cpp
        node_3d.h
        occluder_instance_3d.cpp
        occluder_instance_3d.h
        path_3d.cpp
        path_3d.h
        physical_bone_simulator_3d.cpp
        physical_bone_simulator_3d.h
        reflection_probe.cpp
        reflection_probe.h
        remote_transform_3d.cpp
        remote_transform_3d.h
        skeleton_3d.cpp
        skeleton_3d.h
        skeleton_ik_3d.cpp
        skeleton_ik_3d.h
        skeleton_modifier_3d.cpp
        skeleton_modifier_3d.h
        soft_body_3d.cpp
        soft_body_3d.h
        sprite_3d.cpp
        sprite_3d.h
        velocity_tracker_3d.cpp
        velocity_tracker_3d.h
        visible_on_screen_notifier_3d.cpp
        visible_on_screen_notifier_3d.h
        visual_instance_3d.cpp
        visual_instance_3d.h
        voxel_gi.cpp
        voxel_gi.h
        voxelizer.cpp
        voxelizer.h
        world_environment.cpp
        world_environment.h
        xr_body_modifier_3d.cpp
        xr_body_modifier_3d.h
        xr_face_modifier_3d.cpp
        xr_face_modifier_3d.h
        xr_hand_modifier_3d.cpp
        xr_hand_modifier_3d.h
        xr_nodes.cpp
        xr_nodes.h
)
# [[[end]]]
target_link_libraries(godot-scene-3d PUBLIC godot-os godot-templates godot-thirdparty-misc zlib)
