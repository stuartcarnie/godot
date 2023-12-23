godot_add_library(thorvg FOLDER thirdparty STATIC)
target_sources(thorvg PRIVATE
        # common
        src/common/tvgBezier.cpp
        src/common/tvgCompressor.cpp
        src/common/tvgMath.cpp
        src/common/tvgStr.cpp
        # SVG parser
        src/loaders/svg/tvgSvgCssStyle.cpp
        src/loaders/svg/tvgSvgLoader.cpp
        src/loaders/svg/tvgSvgPath.cpp
        src/loaders/svg/tvgSvgSceneBuilder.cpp
        src/loaders/svg/tvgSvgUtil.cpp
        src/loaders/svg/tvgXmlParser.cpp
        src/loaders/raw/tvgRawLoader.cpp
        src/loaders/external_png/tvgPngLoader.cpp
        src/loaders/jpg/tvgJpgd.cpp
        src/loaders/jpg/tvgJpgLoader.cpp
        # renderer common
        src/renderer/tvgAccessor.cpp
        # src/renderer/tvgAnimation.cpp
        src/renderer/tvgCanvas.cpp
        src/renderer/tvgFill.cpp
        # src/renderer/tvgGlCanvas.cpp
        src/renderer/tvgInitializer.cpp
        src/renderer/tvgLoader.cpp
        src/renderer/tvgPaint.cpp
        src/renderer/tvgPicture.cpp
        src/renderer/tvgRender.cpp
        # src/renderer/tvgSaver.cpp
        src/renderer/tvgScene.cpp
        src/renderer/tvgShape.cpp
        src/renderer/tvgSwCanvas.cpp
        src/renderer/tvgTaskScheduler.cpp
        src/renderer/tvgText.cpp
        # src/renderer/tvgWgCanvas.cpp
        # renderer sw_engine
        src/renderer/sw_engine/tvgSwFill.cpp
        src/renderer/sw_engine/tvgSwImage.cpp
        src/renderer/sw_engine/tvgSwMath.cpp
        src/renderer/sw_engine/tvgSwMemPool.cpp
        src/renderer/sw_engine/tvgSwRaster.cpp
        src/renderer/sw_engine/tvgSwRenderer.cpp
        src/renderer/sw_engine/tvgSwRle.cpp
        src/renderer/sw_engine/tvgSwShape.cpp
        src/renderer/sw_engine/tvgSwStroke.cpp
)
target_include_directories(thorvg PUBLIC inc)
target_include_directories(thorvg PUBLIC AFTER
        src/common
        src/loaders/svg
        src/renderer
        src/renderer/sw_engine
        src/loaders/raw
        src/loaders/external_png
        src/loaders/jpg
)
target_link_libraries(thorvg PUBLIC png)
target_compile_definitions(thorvg PUBLIC TVG_STATIC)
