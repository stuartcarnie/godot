add_subdirectory(joints)

godot_add_library(godot-server-physics-3D FOLDER servers STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-server-physics-3D PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-server-physics-3D PRIVATE
        gjk_epa.cpp
        gjk_epa.h
        godot_area_3d.cpp
        godot_area_3d.h
        godot_area_pair_3d.cpp
        godot_area_pair_3d.h
        godot_body_3d.cpp
        godot_body_3d.h
        godot_body_direct_state_3d.cpp
        godot_body_direct_state_3d.h
        godot_body_pair_3d.cpp
        godot_body_pair_3d.h
        godot_broad_phase_3d.cpp
        godot_broad_phase_3d.h
        godot_broad_phase_3d_bvh.cpp
        godot_broad_phase_3d_bvh.h
        godot_collision_object_3d.cpp
        godot_collision_object_3d.h
        godot_collision_solver_3d.cpp
        godot_collision_solver_3d.h
        godot_collision_solver_3d_sat.cpp
        godot_collision_solver_3d_sat.h
        godot_constraint_3d.h
        godot_joint_3d.h
        godot_physics_server_3d.cpp
        godot_physics_server_3d.h
        godot_shape_3d.cpp
        godot_shape_3d.h
        godot_soft_body_3d.cpp
        godot_soft_body_3d.h
        godot_space_3d.cpp
        godot_space_3d.h
        godot_step_3d.cpp
        godot_step_3d.h
)
# [[[end]]]

target_link_libraries(godot-server-physics-3D PUBLIC godot-math)
