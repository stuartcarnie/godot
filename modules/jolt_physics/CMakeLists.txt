godot_add_library(godot-module-jolt-physics FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-jolt-physics PRIVATE')
#   for file in list_files(cog, ['.', 'joints', 'misc', 'objects', 'shapes', 'spaces']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-jolt-physics PRIVATE
        joints/jolt_cone_twist_joint_3d.cpp
        joints/jolt_cone_twist_joint_3d.h
        joints/jolt_generic_6dof_joint_3d.cpp
        joints/jolt_generic_6dof_joint_3d.h
        joints/jolt_hinge_joint_3d.cpp
        joints/jolt_hinge_joint_3d.h
        joints/jolt_joint_3d.cpp
        joints/jolt_joint_3d.h
        joints/jolt_pin_joint_3d.cpp
        joints/jolt_pin_joint_3d.h
        joints/jolt_slider_joint_3d.cpp
        joints/jolt_slider_joint_3d.h
        jolt_globals.cpp
        jolt_globals.h
        jolt_physics_server_3d.cpp
        jolt_physics_server_3d.h
        jolt_project_settings.cpp
        jolt_project_settings.h
        misc/jolt_math_funcs.cpp
        misc/jolt_math_funcs.h
        misc/jolt_stream_wrappers.h
        misc/jolt_type_conversions.h
        objects/jolt_area_3d.cpp
        objects/jolt_area_3d.h
        objects/jolt_body_3d.cpp
        objects/jolt_body_3d.h
        objects/jolt_group_filter.cpp
        objects/jolt_group_filter.h
        objects/jolt_object_3d.cpp
        objects/jolt_object_3d.h
        objects/jolt_physics_direct_body_state_3d.cpp
        objects/jolt_physics_direct_body_state_3d.h
        objects/jolt_shaped_object_3d.cpp
        objects/jolt_shaped_object_3d.h
        objects/jolt_soft_body_3d.cpp
        objects/jolt_soft_body_3d.h
        register_types.cpp
        register_types.h
        shapes/jolt_box_shape_3d.cpp
        shapes/jolt_box_shape_3d.h
        shapes/jolt_capsule_shape_3d.cpp
        shapes/jolt_capsule_shape_3d.h
        shapes/jolt_concave_polygon_shape_3d.cpp
        shapes/jolt_concave_polygon_shape_3d.h
        shapes/jolt_convex_polygon_shape_3d.cpp
        shapes/jolt_convex_polygon_shape_3d.h
        shapes/jolt_custom_decorated_shape.h
        shapes/jolt_custom_double_sided_shape.cpp
        shapes/jolt_custom_double_sided_shape.h
        shapes/jolt_custom_motion_shape.cpp
        shapes/jolt_custom_motion_shape.h
        shapes/jolt_custom_ray_shape.cpp
        shapes/jolt_custom_ray_shape.h
        shapes/jolt_custom_shape_type.h
        shapes/jolt_custom_user_data_shape.cpp
        shapes/jolt_custom_user_data_shape.h
        shapes/jolt_cylinder_shape_3d.cpp
        shapes/jolt_cylinder_shape_3d.h
        shapes/jolt_height_map_shape_3d.cpp
        shapes/jolt_height_map_shape_3d.h
        shapes/jolt_separation_ray_shape_3d.cpp
        shapes/jolt_separation_ray_shape_3d.h
        shapes/jolt_shape_3d.cpp
        shapes/jolt_shape_3d.h
        shapes/jolt_shape_instance_3d.cpp
        shapes/jolt_shape_instance_3d.h
        shapes/jolt_sphere_shape_3d.cpp
        shapes/jolt_sphere_shape_3d.h
        shapes/jolt_world_boundary_shape_3d.cpp
        shapes/jolt_world_boundary_shape_3d.h
        spaces/jolt_body_activation_listener_3d.cpp
        spaces/jolt_body_activation_listener_3d.h
        spaces/jolt_broad_phase_layer.h
        spaces/jolt_contact_listener_3d.cpp
        spaces/jolt_contact_listener_3d.h
        spaces/jolt_job_system.cpp
        spaces/jolt_job_system.h
        spaces/jolt_layers.cpp
        spaces/jolt_layers.h
        spaces/jolt_motion_filter_3d.cpp
        spaces/jolt_motion_filter_3d.h
        spaces/jolt_physics_direct_space_state_3d.cpp
        spaces/jolt_physics_direct_space_state_3d.h
        spaces/jolt_query_collectors.h
        spaces/jolt_query_filter_3d.cpp
        spaces/jolt_query_filter_3d.h
        spaces/jolt_space_3d.cpp
        spaces/jolt_space_3d.h
        spaces/jolt_temp_allocator.cpp
        spaces/jolt_temp_allocator.h
)
# [[[end]]]
target_link_libraries(godot-module-jolt-physics PUBLIC core jolt-physics)
target_compile_definitions(godot-module-jolt-physics
        PUBLIC
        $<${WANT_EDITOR}:JPH_DEBUG_RENDERER>
        $<${WANT_DEV_BUILD}:JPH_ENABLE_ASSERTS>
        $<${WANT_DOUBLE_PRECISION}:JPH_DOUBLE_PRECISION>
)

group_target_sources(godot-module-jolt-physics)
