set(LOGO_SVG_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/logo.svg)
set(LOGO_SVG_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/platform/macos/export/logo_svg.gen.h)
generate_export_icon(macos logo ${LOGO_SVG_SOURCE} ${LOGO_SVG_OUTPUT})
set(RUN_SVG_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/run_icon.svg)
set(RUN_SVG_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/platform/macos/export/run_icon_svg.gen.h)
generate_export_icon(macos run_icon ${RUN_SVG_SOURCE} ${RUN_SVG_OUTPUT})

godot_add_library(platform-macos-export FOLDER macos
        # generated
        ${LOGO_SVG_OUTPUT}
        ${RUN_SVG_OUTPUT}
)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(platform-macos-export PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(platform-macos-export PRIVATE
        codesign.cpp
        codesign.h
        export.cpp
        export.h
        export_plugin.cpp
        export_plugin.h
        lipo.cpp
        lipo.h
        macho.cpp
        macho.h
)
# [[[end]]]
target_link_libraries(platform-macos-export PRIVATE godot-config)
target_include_directories(platform-macos-export
        PRIVATE
        ${CMAKE_BINARY_DIR}/include/platform/macos/export
)

