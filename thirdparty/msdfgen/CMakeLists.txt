godot_add_library(msdfgen FOLDER thirdparty STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(msdfgen PRIVATE')
#   for file in list_files(cog, ['.', 'core']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(msdfgen PRIVATE
        core/Bitmap.h
        core/Bitmap.hpp
        core/BitmapRef.hpp
        core/Contour.cpp
        core/Contour.h
        core/EdgeColor.h
        core/EdgeHolder.cpp
        core/EdgeHolder.h
        core/MSDFErrorCorrection.cpp
        core/MSDFErrorCorrection.h
        core/Projection.cpp
        core/Projection.h
        core/Scanline.cpp
        core/Scanline.h
        core/Shape.cpp
        core/Shape.h
        core/ShapeDistanceFinder.h
        core/ShapeDistanceFinder.hpp
        core/SignedDistance.cpp
        core/SignedDistance.h
        core/Vector2.cpp
        core/Vector2.h
        core/arithmetics.hpp
        core/bitmap-interpolation.hpp
        core/contour-combiners.cpp
        core/contour-combiners.h
        core/edge-coloring.cpp
        core/edge-coloring.h
        core/edge-segments.cpp
        core/edge-segments.h
        core/edge-selectors.cpp
        core/edge-selectors.h
        core/equation-solver.cpp
        core/equation-solver.h
        core/generator-config.h
        core/msdf-error-correction.cpp
        core/msdf-error-correction.h
        core/msdfgen.cpp
        core/pixel-conversion.hpp
        core/rasterization.cpp
        core/rasterization.h
        core/render-sdf.cpp
        core/render-sdf.h
        core/save-bmp.cpp
        core/save-bmp.h
        core/save-tiff.cpp
        core/save-tiff.h
        core/sdf-error-estimation.cpp
        core/sdf-error-estimation.h
        core/shape-description.cpp
        core/shape-description.h
        msdfgen.h
)
# [[[end]]]
target_include_directories(msdfgen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
