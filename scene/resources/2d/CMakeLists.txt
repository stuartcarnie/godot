godot_add_library(godot-scene-resources-2d FOLDER scene STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-scene-resources-2d PRIVATE')
#   for file in list_files(cog, ['.', 'skeleton']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-scene-resources-2d PRIVATE
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
        skeleton/skeleton_modification_2d.cpp
        skeleton/skeleton_modification_2d.h
        skeleton/skeleton_modification_2d_ccdik.cpp
        skeleton/skeleton_modification_2d_ccdik.h
        skeleton/skeleton_modification_2d_fabrik.cpp
        skeleton/skeleton_modification_2d_fabrik.h
        skeleton/skeleton_modification_2d_jiggle.cpp
        skeleton/skeleton_modification_2d_jiggle.h
        skeleton/skeleton_modification_2d_lookat.cpp
        skeleton/skeleton_modification_2d_lookat.h
        skeleton/skeleton_modification_2d_physicalbones.cpp
        skeleton/skeleton_modification_2d_physicalbones.h
        skeleton/skeleton_modification_2d_stackholder.cpp
        skeleton/skeleton_modification_2d_stackholder.h
        skeleton/skeleton_modification_2d_twoboneik.cpp
        skeleton/skeleton_modification_2d_twoboneik.h
        skeleton/skeleton_modification_stack_2d.cpp
        skeleton/skeleton_modification_stack_2d.h
        tile_set.cpp
        tile_set.h
        world_boundary_shape_2d.cpp
        world_boundary_shape_2d.h
)
# [[[end]]]
target_link_libraries(godot-scene-resources-2d PUBLIC godot-core)
