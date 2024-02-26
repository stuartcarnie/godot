godot_add_library(godot-scene-resources-3d FOLDER scene STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-scene-resources-3d PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-scene-resources-3d PRIVATE
        box_shape_3d.cpp
        box_shape_3d.h
        capsule_shape_3d.cpp
        capsule_shape_3d.h
        concave_polygon_shape_3d.cpp
        concave_polygon_shape_3d.h
        convex_polygon_shape_3d.cpp
        convex_polygon_shape_3d.h
        cylinder_shape_3d.cpp
        cylinder_shape_3d.h
        fog_material.cpp
        fog_material.h
        height_map_shape_3d.cpp
        height_map_shape_3d.h
        importer_mesh.cpp
        importer_mesh.h
        mesh_library.cpp
        mesh_library.h
        primitive_meshes.cpp
        primitive_meshes.h
        separation_ray_shape_3d.cpp
        separation_ray_shape_3d.h
        shape_3d.cpp
        shape_3d.h
        skin.cpp
        skin.h
        sky_material.cpp
        sky_material.h
        sphere_shape_3d.cpp
        sphere_shape_3d.h
        world_3d.cpp
        world_3d.h
        world_boundary_shape_3d.cpp
        world_boundary_shape_3d.h
)
# [[[end]]]
target_link_libraries(godot-scene-resources-3d PUBLIC godot-core)
