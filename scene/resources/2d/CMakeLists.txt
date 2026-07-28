add_subdirectory(skeleton)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(scene PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(scene PRIVATE
        capsule_shape_2d.cpp
        capsule_shape_2d.h
        circle_shape_2d.cpp
        circle_shape_2d.h
        concave_polygon_shape_2d.cpp
        concave_polygon_shape_2d.h
        convex_polygon_shape_2d.cpp
        convex_polygon_shape_2d.h
        navigation_mesh_source_geometry_data_2d.cpp
        navigation_mesh_source_geometry_data_2d.h
        navigation_polygon.cpp
        navigation_polygon.h
        polygon_path_finder.cpp
        polygon_path_finder.h
        rectangle_shape_2d.cpp
        rectangle_shape_2d.h
        segment_shape_2d.cpp
        segment_shape_2d.h
        separation_ray_shape_2d.cpp
        separation_ray_shape_2d.h
        shape_2d.cpp
        shape_2d.h
        world_boundary_shape_2d.cpp
        world_boundary_shape_2d.h
)
# [[[end]]]
