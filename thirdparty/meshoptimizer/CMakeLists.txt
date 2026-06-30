godot_add_library(meshoptimizer FOLDER thirdparty STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(meshoptimizer PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(meshoptimizer PRIVATE
        allocator.cpp
        clusterizer.cpp
        indexanalyzer.cpp
        indexcodec.cpp
        indexgenerator.cpp
        meshletcodec.cpp
        meshletutils.cpp
        meshoptimizer.h
        opacitymap.cpp
        overdrawoptimizer.cpp
        partition.cpp
        quantization.cpp
        rasterizer.cpp
        simplifier.cpp
        spatialorder.cpp
        stripifier.cpp
        tangentspace.cpp
        vcacheoptimizer.cpp
        vertexcodec.cpp
        vertexfilter.cpp
        vfetchoptimizer.cpp
)
# [[[end]]]
