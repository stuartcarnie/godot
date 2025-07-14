 add_subdirectory(physics)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        audio_listener_3d_gizmo_plugin.cpp
        audio_listener_3d_gizmo_plugin.h
        audio_stream_player_3d_gizmo_plugin.cpp
        audio_stream_player_3d_gizmo_plugin.h
        camera_3d_gizmo_plugin.cpp
        camera_3d_gizmo_plugin.h
        cpu_particles_3d_gizmo_plugin.cpp
        cpu_particles_3d_gizmo_plugin.h
        decal_gizmo_plugin.cpp
        decal_gizmo_plugin.h
        fog_volume_gizmo_plugin.cpp
        fog_volume_gizmo_plugin.h
        geometry_instance_3d_gizmo_plugin.cpp
        geometry_instance_3d_gizmo_plugin.h
        gizmo_3d_helper.cpp
        gizmo_3d_helper.h
        gpu_particles_3d_gizmo_plugin.cpp
        gpu_particles_3d_gizmo_plugin.h
        gpu_particles_collision_3d_gizmo_plugin.cpp
        gpu_particles_collision_3d_gizmo_plugin.h
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
        occluder_instance_3d_gizmo_plugin.cpp
        occluder_instance_3d_gizmo_plugin.h
        particles_3d_emission_shape_gizmo_plugin.cpp
        particles_3d_emission_shape_gizmo_plugin.h
        reflection_probe_gizmo_plugin.cpp
        reflection_probe_gizmo_plugin.h
        sprite_base_3d_gizmo_plugin.cpp
        sprite_base_3d_gizmo_plugin.h
        visible_on_screen_notifier_3d_gizmo_plugin.cpp
        visible_on_screen_notifier_3d_gizmo_plugin.h
        voxel_gi_gizmo_plugin.cpp
        voxel_gi_gizmo_plugin.h
)
# [[[end]]]
