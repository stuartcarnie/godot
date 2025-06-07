add_subdirectory(navigation)
add_subdirectory(physics)

if (NOT DISABLE_XR)
    add_subdirectory(xr)
endif ()

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(scene PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(scene PRIVATE
        aim_modifier_3d.cpp
        aim_modifier_3d.h
        audio_listener_3d.cpp
        audio_listener_3d.h
        audio_stream_player_3d.cpp
        audio_stream_player_3d.h
        bone_attachment_3d.cpp
        bone_attachment_3d.h
        bone_constraint_3d.cpp
        bone_constraint_3d.h
        camera_3d.cpp
        camera_3d.h
        convert_transform_modifier_3d.cpp
        convert_transform_modifier_3d.h
        copy_transform_modifier_3d.cpp
        copy_transform_modifier_3d.h
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
        look_at_modifier_3d.cpp
        look_at_modifier_3d.h
        marker_3d.cpp
        marker_3d.h
        mesh_instance_3d.cpp
        mesh_instance_3d.h
        modifier_bone_target_3d.cpp
        modifier_bone_target_3d.h
        multimesh_instance_3d.cpp
        multimesh_instance_3d.h
        node_3d.cpp
        node_3d.h
        occluder_instance_3d.cpp
        occluder_instance_3d.h
        path_3d.cpp
        path_3d.h
        reflection_probe.cpp
        reflection_probe.h
        remote_transform_3d.cpp
        remote_transform_3d.h
        retarget_modifier_3d.cpp
        retarget_modifier_3d.h
        skeleton_3d.cpp
        skeleton_3d.h
        skeleton_ik_3d.cpp
        skeleton_ik_3d.h
        skeleton_modifier_3d.cpp
        skeleton_modifier_3d.h
        spring_bone_collision_3d.cpp
        spring_bone_collision_3d.h
        spring_bone_collision_capsule_3d.cpp
        spring_bone_collision_capsule_3d.h
        spring_bone_collision_plane_3d.cpp
        spring_bone_collision_plane_3d.h
        spring_bone_collision_sphere_3d.cpp
        spring_bone_collision_sphere_3d.h
        spring_bone_simulator_3d.cpp
        spring_bone_simulator_3d.h
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
)
# [[[end]]]
target_link_libraries(scene PUBLIC godot-thirdparty-misc zlib)
