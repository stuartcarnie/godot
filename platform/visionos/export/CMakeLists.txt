set(LOGO_SVG_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/logo.svg)
set(LOGO_SVG_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/platform/visionos/export/logo_svg.gen.h)
generate_export_icon(visionos logo ${LOGO_SVG_SOURCE} ${LOGO_SVG_OUTPUT})
set(RUN_SVG_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/run_icon.svg)
set(RUN_SVG_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/platform/visionos/export/run_icon_svg.gen.h)
generate_export_icon(visionos run_icon ${RUN_SVG_SOURCE} ${RUN_SVG_OUTPUT})

godot_add_library(platform-visionos-export FOLDER macos
        # generated
        ${LOGO_SVG_OUTPUT}
        ${RUN_SVG_OUTPUT}
)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(platform-visionos-export PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(platform-visionos-export PRIVATE
        export.cpp
        export.h
        export_plugin.cpp
        export_plugin.h
)
# [[[end]]]
target_link_libraries(platform-visionos-export PRIVATE core)
target_include_directories(platform-visionos-export
        PRIVATE
        ${CMAKE_BINARY_DIR}/include/platform/visionos/export
)

