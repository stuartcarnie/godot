godot_add_library(recast-navigation FOLDER thirdparty STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(recast-navigation PRIVATE')
#   for file in list_files(cog, 'Recast', recursive=True):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(recast-navigation PRIVATE
        Recast/Include/Recast.h
        Recast/Include/RecastAlloc.h
        Recast/Include/RecastAssert.h
        Recast/Source/Recast.cpp
        Recast/Source/RecastAlloc.cpp
        Recast/Source/RecastArea.cpp
        Recast/Source/RecastAssert.cpp
        Recast/Source/RecastContour.cpp
        Recast/Source/RecastFilter.cpp
        Recast/Source/RecastLayers.cpp
        Recast/Source/RecastMesh.cpp
        Recast/Source/RecastMeshDetail.cpp
        Recast/Source/RecastRasterization.cpp
        Recast/Source/RecastRegion.cpp
)
# [[[end]]]
target_include_directories(recast-navigation PUBLIC Recast/Include)
