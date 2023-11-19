godot_add_library(squish FOLDER thirdparty STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(squish PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(squish PRIVATE
        alpha.cpp
        alpha.h
        clusterfit.cpp
        clusterfit.h
        colourblock.cpp
        colourblock.h
        colourfit.cpp
        colourfit.h
        colourset.cpp
        colourset.h
        config.h
        maths.cpp
        maths.h
        rangefit.cpp
        rangefit.h
        simd.h
        simd_float.h
        simd_sse.h
        simd_ve.h
        singlecolourfit.cpp
        singlecolourfit.h
        squish.cpp
        squish.h
)
# [[[end]]]
target_include_directories(squish PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
