godot_add_library(msdfgen FOLDER thirdparty STATIC)

target_sources(msdfgen PRIVATE
        core/Contour.cpp
        core/DistanceMapping.cpp
        core/EdgeHolder.cpp
        core/MSDFErrorCorrection.cpp
        core/Projection.cpp
        core/Scanline.cpp
        core/Shape.cpp
        core/contour-combiners.cpp
        core/edge-coloring.cpp
        core/edge-segments.cpp
        core/edge-selectors.cpp
        core/equation-solver.cpp
        # core/export-svg.cpp
        core/msdf-error-correction.cpp
        core/msdfgen.cpp
        core/rasterization.cpp
        core/render-sdf.cpp
        # core/save-bmp.cpp
        # core/save-fl32.cpp
        # core/save-rgba.cpp
        # core/save-tiff.cpp
        core/sdf-error-estimation.cpp
        core/shape-description.cpp
)
target_include_directories(msdfgen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(msdfgen PUBLIC MSDFGEN_PUBLIC=)
