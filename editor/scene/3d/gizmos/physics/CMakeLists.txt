# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        collision_object_3d_gizmo_plugin.cpp
        collision_object_3d_gizmo_plugin.h
        collision_polygon_3d_gizmo_plugin.cpp
        collision_polygon_3d_gizmo_plugin.h
        collision_shape_3d_gizmo_plugin.cpp
        collision_shape_3d_gizmo_plugin.h
        joint_3d_gizmo_plugin.cpp
        joint_3d_gizmo_plugin.h
        physics_bone_3d_gizmo_plugin.cpp
        physics_bone_3d_gizmo_plugin.h
        ray_cast_3d_gizmo_plugin.cpp
        ray_cast_3d_gizmo_plugin.h
        shape_cast_3d_gizmo_plugin.cpp
        shape_cast_3d_gizmo_plugin.h
        soft_body_3d_gizmo_plugin.cpp
        soft_body_3d_gizmo_plugin.h
        spring_arm_3d_gizmo_plugin.cpp
        spring_arm_3d_gizmo_plugin.h
        spring_bone_3d_gizmo_plugin.cpp
        spring_bone_3d_gizmo_plugin.h
        vehicle_body_3d_gizmo_plugin.cpp
        vehicle_body_3d_gizmo_plugin.h
)
# [[[end]]]
