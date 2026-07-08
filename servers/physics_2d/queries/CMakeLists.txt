# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(servers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(servers PRIVATE
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
