add_subdirectory(gizmos)
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
        camera_3d_editor_plugin.cpp
        camera_3d_editor_plugin.h
        lightmap_gi_editor_plugin.cpp
        lightmap_gi_editor_plugin.h
        mesh_editor_plugin.cpp
        mesh_editor_plugin.h
        mesh_instance_3d_editor_plugin.cpp
        mesh_instance_3d_editor_plugin.h
        mesh_library_editor_plugin.cpp
        mesh_library_editor_plugin.h
        multimesh_editor_plugin.cpp
        multimesh_editor_plugin.h
        node_3d_editor_gizmos.cpp
        node_3d_editor_gizmos.h
        node_3d_editor_plugin.cpp
        node_3d_editor_plugin.h
        occluder_instance_3d_editor_plugin.cpp
        occluder_instance_3d_editor_plugin.h
        particles_3d_editor_plugin.cpp
        particles_3d_editor_plugin.h
        path_3d_editor_plugin.cpp
        path_3d_editor_plugin.h
        polygon_3d_editor_plugin.cpp
        polygon_3d_editor_plugin.h
        root_motion_editor_plugin.cpp
        root_motion_editor_plugin.h
        skeleton_3d_editor_plugin.cpp
        skeleton_3d_editor_plugin.h
        skeleton_ik_3d_editor_plugin.cpp
        skeleton_ik_3d_editor_plugin.h
        voxel_gi_editor_plugin.cpp
        voxel_gi_editor_plugin.h
)
# [[[end]]]
