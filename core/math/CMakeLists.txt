godot_add_library(godot-math FOLDER core STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-math PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-math PRIVATE
        a_star.cpp
        a_star.h
        a_star_grid_2d.cpp
        a_star_grid_2d.h
        aabb.cpp
        aabb.h
        audio_frame.h
        basis.cpp
        basis.h
        bvh.h
        bvh_abb.h
        bvh_tree.h
        color.cpp
        color.h
        convex_hull.cpp
        convex_hull.h
        delaunay_2d.h
        delaunay_3d.h
        disjoint_set.h
        dynamic_bvh.cpp
        dynamic_bvh.h
        expression.cpp
        expression.h
        face3.cpp
        face3.h
        geometry_2d.cpp
        geometry_2d.h
        geometry_3d.cpp
        geometry_3d.h
        math_defs.h
        math_fieldwise.cpp
        math_fieldwise.h
        math_funcs.cpp
        math_funcs.h
        plane.cpp
        plane.h
        projection.cpp
        projection.h
        quaternion.cpp
        quaternion.h
        quick_hull.cpp
        quick_hull.h
        random_number_generator.cpp
        random_number_generator.h
        random_pcg.cpp
        random_pcg.h
        rect2.cpp
        rect2.h
        rect2i.cpp
        rect2i.h
        static_raycaster.cpp
        static_raycaster.h
        transform_2d.cpp
        transform_2d.h
        transform_3d.cpp
        transform_3d.h
        triangle_mesh.cpp
        triangle_mesh.h
        triangulate.cpp
        triangulate.h
        vector2.cpp
        vector2.h
        vector2i.cpp
        vector2i.h
        vector3.cpp
        vector3.h
        vector3i.cpp
        vector3i.h
        vector4.cpp
        vector4.h
        vector4i.cpp
        vector4i.h
)
# [[[end]]]
target_include_directories(godot-math PUBLIC ${GODOT_ENGINE_ROOT_DIRECTORY})
target_link_libraries(godot-math PUBLIC godot-object)

target_sources(godot-math PUBLIC
        FILE_SET HEADERS
        FILES
        a_star.h
        a_star_grid_2d.h
        aabb.h
        audio_frame.h
        basis.h
        bvh.h
        bvh_abb.h
        bvh_tree.h
        color.h
        convex_hull.h
        delaunay_2d.h
        delaunay_3d.h
        disjoint_set.h
        dynamic_bvh.h
        expression.h
        face3.h
        geometry_2d.h
        geometry_3d.h
        math_defs.h
        math_fieldwise.h
        math_funcs.h
        plane.h
        projection.h
        quaternion.h
        quick_hull.h
        random_number_generator.h
        random_pcg.h
        rect2.h
        rect2i.h
        static_raycaster.h
        transform_2d.h
        transform_3d.h
        triangle_mesh.h
        triangulate.h
        vector2.h
        vector2i.h
        vector3.h
        vector3i.h
        vector4.h
        vector4i.h
)
