# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(servers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(servers PRIVATE
        physics_cast_motion_result_2d.cpp
        physics_cast_motion_result_2d.h
        physics_collide_shape_result_2d.cpp
        physics_collide_shape_result_2d.h
        physics_get_rest_info_result_2d.cpp
        physics_get_rest_info_result_2d.h
        physics_intersect_point_result_2d.cpp
        physics_intersect_point_result_2d.h
        physics_intersect_ray_result_2d.cpp
        physics_intersect_ray_result_2d.h
        physics_intersect_shape_result_2d.cpp
        physics_intersect_shape_result_2d.h
        physics_point_query_parameters_2d.cpp
        physics_point_query_parameters_2d.h
        physics_ray_query_parameters_2d.cpp
        physics_ray_query_parameters_2d.h
        physics_shape_query_parameters_2d.cpp
        physics_shape_query_parameters_2d.h
        physics_testmotion_query_parameters_2d.cpp
        physics_testmotion_query_parameters_2d.h
        physics_testmotion_query_result_2d.cpp
        physics_testmotion_query_result_2d.h
)
# [[[end]]]
