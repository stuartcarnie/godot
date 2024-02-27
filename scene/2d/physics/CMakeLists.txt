add_subdirectory(joints)

godot_add_library(godot-scene-2d-physics FOLDER scene STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-scene-2d-physics PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-scene-2d-physics PRIVATE
        animatable_body_2d.cpp
        animatable_body_2d.h
        area_2d.cpp
        area_2d.h
        character_body_2d.cpp
        character_body_2d.h
        collision_object_2d.cpp
        collision_object_2d.h
        collision_polygon_2d.cpp
        collision_polygon_2d.h
        collision_shape_2d.cpp
        collision_shape_2d.h
        kinematic_collision_2d.cpp
        kinematic_collision_2d.h
        physical_bone_2d.cpp
        physical_bone_2d.h
        physics_body_2d.cpp
        physics_body_2d.h
        ray_cast_2d.cpp
        ray_cast_2d.h
        rigid_body_2d.cpp
        rigid_body_2d.h
        shape_cast_2d.cpp
        shape_cast_2d.h
        static_body_2d.cpp
        static_body_2d.h
)
# [[[end]]]
target_link_libraries(godot-scene-2d-physics PRIVATE godot-config)

