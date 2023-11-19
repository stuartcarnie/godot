godot_add_library(vhacd FOLDER thirdparty STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(vhacd PRIVATE')
#   for file in list_files(cog, ['inc', 'public', 'src']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(vhacd PRIVATE
        inc/FloatMath.h
        inc/btAlignedAllocator.h
        inc/btAlignedObjectArray.h
        inc/btConvexHullComputer.h
        inc/btMinMax.h
        inc/btScalar.h
        inc/btVector3.h
        inc/vhacdCircularList.h
        inc/vhacdICHull.h
        inc/vhacdManifoldMesh.h
        inc/vhacdMesh.h
        inc/vhacdMutex.h
        inc/vhacdRaycastMesh.h
        inc/vhacdSArray.h
        inc/vhacdTimer.h
        inc/vhacdVHACD.h
        inc/vhacdVector.h
        inc/vhacdVolume.h
        public/VHACD.h
        src/FloatMath.cpp
        src/VHACD-ASYNC.cpp
        src/VHACD.cpp
        src/btAlignedAllocator.cpp
        src/btConvexHullComputer.cpp
        src/vhacdICHull.cpp
        src/vhacdManifoldMesh.cpp
        src/vhacdMesh.cpp
        src/vhacdRaycastMesh.cpp
        src/vhacdVolume.cpp
)
# [[[end]]]
target_include_directories(vhacd PUBLIC inc)
target_compile_options(vhacd PUBLIC $<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>)