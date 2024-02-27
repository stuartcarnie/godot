add_subdirectory(joints)

godot_add_library(godot-scene-3d-physics FOLDER scene STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-scene-3d-physics PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-scene-3d-physics PRIVATE
        animatable_body_3d.cpp
        animatable_body_3d.h
        area_3d.cpp
        area_3d.h
        character_body_3d.cpp
        character_body_3d.h
        collision_object_3d.cpp
        collision_object_3d.h
        collision_polygon_3d.cpp
        collision_polygon_3d.h
        collision_shape_3d.cpp
        collision_shape_3d.h
        kinematic_collision_3d.cpp
        kinematic_collision_3d.h
        physical_bone_3d.cpp
        physical_bone_3d.h
        physics_body_3d.cpp
        physics_body_3d.h
        ray_cast_3d.cpp
        ray_cast_3d.h
        rigid_body_3d.cpp
        rigid_body_3d.h
        shape_cast_3d.cpp
        shape_cast_3d.h
        spring_arm_3d.cpp
        spring_arm_3d.h
        static_body_3d.cpp
        static_body_3d.h
        vehicle_body_3d.cpp
        vehicle_body_3d.h
)
# [[[end]]]
target_link_libraries(godot-scene-3d-physics PRIVATE godot-config)
