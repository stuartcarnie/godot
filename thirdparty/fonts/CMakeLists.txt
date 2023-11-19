set_abs(GODOT_EDITOR_FONTS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}
        DroidSansFallback.woff2
        DroidSansJapanese.woff2
        JetBrainsMono_Regular.woff2
        NotoNaskhArabicUI_Bold.woff2
        NotoNaskhArabicUI_Regular.woff2
        NotoSansBengaliUI_Bold.woff2
        NotoSansBengaliUI_Regular.woff2
        NotoSansDevanagariUI_Bold.woff2
        NotoSansDevanagariUI_Regular.woff2
        NotoSansGeorgian_Bold.woff2
        NotoSansGeorgian_Regular.woff2
        NotoSansHebrew_Bold.woff2
        NotoSansHebrew_Regular.woff2
        NotoSansMalayalamUI_Bold.woff2
        NotoSansMalayalamUI_Regular.woff2
        NotoSansOriya_Bold.woff2
        NotoSansOriya_Regular.woff2
        NotoSansSinhalaUI_Bold.woff2
        NotoSansSinhalaUI_Regular.woff2
        NotoSansTamilUI_Bold.woff2
        NotoSansTamilUI_Regular.woff2
        NotoSansTeluguUI_Bold.woff2
        NotoSansTeluguUI_Regular.woff2
        NotoSansThai_Bold.woff2
        NotoSansThai_Regular.woff2
        NotoSans_Bold.woff2
        NotoSans_Regular.woff2
        OpenSans_SemiBold.woff2
)
set(GODOT_EDITOR_BUILTIN_FONTS_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/editor/builtin_fonts.gen.h)
add_custom_command(
        OUTPUT ${GODOT_EDITOR_BUILTIN_FONTS_OUTPUT}
        COMMAND ${Python3_EXECUTABLE} ${GODOT_GENERATOR_SCRIPT}
        ARGS godot_editor_builtin_fonts --output ${GODOT_EDITOR_BUILTIN_FONTS_OUTPUT} --input "$<JOIN:${GODOT_EDITOR_FONTS_SOURCE}, >"
        DEPENDS ${_input}
        COMMENT "Generating fonts to ${GODOT_EDITOR_BUILTIN_FONTS_OUTPUT}"
        VERBATIM
)

add_library(fonts INTERFACE ${GODOT_EDITOR_BUILTIN_FONTS_OUTPUT})