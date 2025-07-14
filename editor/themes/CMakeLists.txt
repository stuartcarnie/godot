# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('set_abs(GODOT_EDITOR_FONTS_SOURCE ${GODOT_SOURCE_ROOT_THIRDPARTY_DIR}/fonts')
#   for file in list_files(cog, '../../thirdparty/fonts', exts=['ttf', 'otf', 'woff', 'woff2']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
set_abs(GODOT_EDITOR_FONTS_SOURCE ${GODOT_SOURCE_ROOT_THIRDPARTY_DIR}/fonts
        ../../thirdparty/fonts/DroidSansFallback.woff2
        ../../thirdparty/fonts/DroidSansJapanese.woff2
        ../../thirdparty/fonts/JetBrainsMono_Regular.woff2
        ../../thirdparty/fonts/NotoSansBengaliUI_Bold.woff2
        ../../thirdparty/fonts/NotoSansBengaliUI_Regular.woff2
        ../../thirdparty/fonts/NotoSansDevanagariUI_Bold.woff2
        ../../thirdparty/fonts/NotoSansDevanagariUI_Regular.woff2
        ../../thirdparty/fonts/NotoSansGeorgian_Bold.woff2
        ../../thirdparty/fonts/NotoSansGeorgian_Regular.woff2
        ../../thirdparty/fonts/NotoSansHebrew_Bold.woff2
        ../../thirdparty/fonts/NotoSansHebrew_Regular.woff2
        ../../thirdparty/fonts/NotoSansMalayalamUI_Bold.woff2
        ../../thirdparty/fonts/NotoSansMalayalamUI_Regular.woff2
        ../../thirdparty/fonts/NotoSansOriya_Bold.woff2
        ../../thirdparty/fonts/NotoSansOriya_Regular.woff2
        ../../thirdparty/fonts/NotoSansSinhalaUI_Bold.woff2
        ../../thirdparty/fonts/NotoSansSinhalaUI_Regular.woff2
        ../../thirdparty/fonts/NotoSansTamilUI_Bold.woff2
        ../../thirdparty/fonts/NotoSansTamilUI_Regular.woff2
        ../../thirdparty/fonts/NotoSansTeluguUI_Bold.woff2
        ../../thirdparty/fonts/NotoSansTeluguUI_Regular.woff2
        ../../thirdparty/fonts/NotoSansThai_Bold.woff2
        ../../thirdparty/fonts/NotoSansThai_Regular.woff2
        ../../thirdparty/fonts/NotoSans_Bold.woff2
        ../../thirdparty/fonts/NotoSans_Regular.woff2
        ../../thirdparty/fonts/OpenSans_SemiBold.woff2
        ../../thirdparty/fonts/Vazirmatn_Bold.woff2
        ../../thirdparty/fonts/Vazirmatn_Regular.woff2
)
# [[[end]]]

set(GODOT_EDITOR_BUILTIN_FONTS_HEADER ${CMAKE_BINARY_INCLUDE_DIR}/editor/themes/builtin_fonts.gen.h)
generate_editor_themes_fonts("${GODOT_EDITOR_FONTS_SOURCE}" "${GODOT_EDITOR_BUILTIN_FONTS_HEADER}"
        "${CMAKE_BINARY_INCLUDE_DIR}/include/editor/themes/editor_fonts_cmd.txt")

add_custom_target(editor-themes-generated
        DEPENDS
        ${GODOT_EDITOR_BUILTIN_FONTS_HEADER})
set_target_properties(editor-themes-generated PROPERTIES FOLDER "generated")

add_dependencies(editor editor-themes-generated)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        editor_color_map.cpp
        editor_color_map.h
        editor_fonts.cpp
        editor_fonts.h
        editor_icons.cpp
        editor_icons.h
        editor_scale.cpp
        editor_scale.h
        editor_theme.cpp
        editor_theme.h
        editor_theme_manager.cpp
        editor_theme_manager.h
)
# [[[end]]]
target_sources(editor PRIVATE ${GODOT_EDITOR_BUILTIN_FONTS_HEADER})
target_include_directories(editor PRIVATE ${CMAKE_BINARY_INCLUDE_DIR})

set_target_properties(editor-themes-generated PROPERTIES FOLDER "editor/generated")
group_target_sources(editor-themes-generated)
