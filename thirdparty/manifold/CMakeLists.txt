godot_add_library(manifold FOLDER thirdparty STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(manifold PRIVATE')
#   for file in list_files(cog, ['include/manifold', 'src']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(manifold PRIVATE
        include/manifold/common.h
        include/manifold/linalg.h
        include/manifold/manifold.h
        include/manifold/optional_assert.h
        include/manifold/polygon.h
        include/manifold/vec_view.h
        src/boolean3.cpp
        src/boolean3.h
        src/boolean_result.cpp
        src/collider.h
        src/constructors.cpp
        src/csg_tree.cpp
        src/csg_tree.h
        src/disjoint_sets.h
        src/edge_op.cpp
        src/face_op.cpp
        src/hashtable.h
        src/impl.cpp
        src/impl.h
        src/iters.h
        src/manifold.cpp
        src/mesh_fixes.h
        src/parallel.h
        src/polygon.cpp
        src/properties.cpp
        src/quickhull.cpp
        src/quickhull.h
        src/sdf.cpp
        src/shared.h
        src/smoothing.cpp
        src/sort.cpp
        src/sparse.h
        src/subdivision.cpp
        src/svd.h
        src/tree2d.cpp
        src/tree2d.h
        src/tri_dist.h
        src/utils.h
        src/vec.h
)
# [[[end]]]
target_include_directories(manifold PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(manifold PUBLIC MANIFOLD_PAR=-1)
