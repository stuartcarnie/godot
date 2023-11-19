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
        editor/th.po
        editor/tr.po
        editor/uk.po
        editor/vi.po
        editor/zh_CN.po
        editor/zh_TW.po
)
# [[[end]]]

set(GODOT_EDITOR_TRANSLATIONS_DOCUMENTATION_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/editor/editor_translations.gen.h)
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
        properties/de.po
        properties/es.po
        properties/et.po
        properties/fr.po
        properties/id.po
        properties/it.po
        properties/ja.po
        properties/ko.po
        properties/pl.po
        properties/pt.po
        properties/pt_BR.po
        properties/ru.po
        properties/tr.po
        properties/uk.po
        properties/zh_CN.po
        properties/zh_TW.po
)
# [[[end]]]

set(GODOT_EDITOR_PROPERTIES_TRANSLATIONS_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/editor/property_translations.gen.h)
generate_file(make_editor_properties_translations "${GODOT_PROPERTIES_TRANSLATIONS_SOURCE}" "${GODOT_EDITOR_PROPERTIES_TRANSLATIONS_OUTPUT}")

add_library(godot-editor-translations INTERFACE
        ${GODOT_EDITOR_TRANSLATIONS_DOCUMENTATION_OUTPUT}
        ${GODOT_EDITOR_PROPERTIES_TRANSLATIONS_OUTPUT}
)