godot_add_library(godot-server-physics-2D FOLDER servers STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-server-physics-2D PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-server-physics-2D PRIVATE
        godot_area_2d.cpp
        godot_area_2d.h
        godot_area_pair_2d.cpp
        godot_area_pair_2d.h
        godot_body_2d.cpp
        godot_body_2d.h
        godot_body_direct_state_2d.cpp
        godot_body_direct_state_2d.h
        godot_body_pair_2d.cpp
        godot_body_pair_2d.h
        godot_broad_phase_2d.cpp
        godot_broad_phase_2d.h
        godot_broad_phase_2d_bvh.cpp
        godot_broad_phase_2d_bvh.h
        godot_collision_object_2d.cpp
        godot_collision_object_2d.h
        godot_collision_solver_2d.cpp
        godot_collision_solver_2d.h
        godot_collision_solver_2d_sat.cpp
        godot_collision_solver_2d_sat.h
        godot_constraint_2d.h
        godot_joints_2d.cpp
        godot_joints_2d.h
        godot_physics_server_2d.cpp
        godot_physics_server_2d.h
        godot_shape_2d.cpp
        godot_shape_2d.h
        godot_space_2d.cpp
        godot_space_2d.h
        godot_step_2d.cpp
        godot_step_2d.h
)
# [[[end]]]
target_link_libraries(godot-server-physics-2D PRIVATE godot-math)
