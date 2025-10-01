# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('set_abs(GODOT_EDITOR_TRANSLATIONS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}')
#   for file in list_files(cog, 'editor', exts=['po']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
set_abs(GODOT_EDITOR_TRANSLATIONS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}
        editor/ar.po
        editor/bg.po
        editor/bn.po
        editor/ca.po
        editor/cs.po
        editor/de.po
        editor/el.po
        editor/eo.po
        editor/es.po
        editor/es_AR.po
        editor/et.po
        editor/fa.po
        editor/fi.po
        editor/fr.po
        editor/ga.po
        editor/gl.po
        editor/he.po
        editor/hu.po
        editor/id.po
        editor/it.po
        editor/ja.po
        editor/ka.po
        editor/ko.po
        editor/lv.po
        editor/ms.po
        editor/nb.po
        editor/nl.po
        editor/pl.po
        editor/pt.po
        editor/pt_BR.po
        editor/ro.po
        editor/ru.po
        editor/sk.po
        editor/sv.po
        editor/ta.po
        editor/th.po
        editor/tok.po
        editor/tr.po
        editor/uk.po
        editor/vi.po
        editor/zh_CN.po
        editor/zh_TW.po
)
# [[[end]]]

set(GODOT_EDITOR_TRANSLATIONS_DOCUMENTATION_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/editor/translations/editor_translations.gen.h ${CMAKE_BINARY_INCLUDE_DIR}/editor/translations/editor_translations.gen.cpp)
generate_file(make_editor_translations "${GODOT_EDITOR_TRANSLATIONS_SOURCE}" "${GODOT_EDITOR_TRANSLATIONS_DOCUMENTATION_OUTPUT}")

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('set_abs(GODOT_PROPERTIES_TRANSLATIONS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}')
#   for file in list_files(cog, 'properties', exts=['po']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
set_abs(GODOT_PROPERTIES_TRANSLATIONS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}
        properties/ar.po
        properties/cs.po
        properties/de.po
        properties/es.po
        properties/et.po
        properties/fa.po
        properties/fr.po
        properties/ga.po
        properties/hi.po
        properties/id.po
        properties/it.po
        properties/ja.po
        properties/ka.po
        properties/ko.po
        properties/pl.po
        properties/pt.po
        properties/pt_BR.po
        properties/ru.po
        properties/sv.po
        properties/ta.po
        properties/tr.po
        properties/uk.po
        properties/vi.po
        properties/zh_CN.po
        properties/zh_TW.po
)
# [[[end]]]

set(GODOT_EDITOR_PROPERTIES_TRANSLATIONS_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/editor/translations/property_translations.gen.h ${CMAKE_BINARY_INCLUDE_DIR}/editor/translations/property_translations.gen.cpp)
generate_file(make_editor_translations "${GODOT_PROPERTIES_TRANSLATIONS_SOURCE}" "${GODOT_EDITOR_PROPERTIES_TRANSLATIONS_OUTPUT}")

add_custom_target(editor-translations-generated
        DEPENDS
        ${GODOT_EDITOR_TRANSLATIONS_DOCUMENTATION_OUTPUT}
        ${GODOT_EDITOR_PROPERTIES_TRANSLATIONS_OUTPUT})
set_target_properties(editor-translations-generated PROPERTIES FOLDER "generated")

add_dependencies(editor editor-translations-generated)

target_sources(editor
        INTERFACE
        ${GODOT_EDITOR_TRANSLATIONS_DOCUMENTATION_OUTPUT}
        ${GODOT_EDITOR_PROPERTIES_TRANSLATIONS_OUTPUT}
)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        editor_locale_dialog.cpp
        editor_locale_dialog.h
        editor_translation.cpp
        editor_translation.h
        editor_translation_parser.cpp
        editor_translation_parser.h
        editor_translation_preview_button.cpp
        editor_translation_preview_button.h
        editor_translation_preview_menu.cpp
        editor_translation_preview_menu.h
        localization_editor.cpp
        localization_editor.h
        packed_scene_translation_parser_plugin.cpp
        packed_scene_translation_parser_plugin.h
        pot_generator.cpp
        pot_generator.h
)
# [[[end]]]
