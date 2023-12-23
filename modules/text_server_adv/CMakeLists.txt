set(ICU4C_DAT_NAME icudt74l.dat)
set(ICU_FONT_DATA_SOURCE ${GODOT_SOURCE_ROOT_THIRDPARTY_DIR}/icu4c/${ICU4C_DAT_NAME})
set(ICU_FONT_DATA_DESTINATION ${CMAKE_BINARY_INCLUDE_DIR}/thirdparty/icu4c/icudata.gen.h)
generate_file(make_icu_data ${ICU_FONT_DATA_SOURCE} ${ICU_FONT_DATA_DESTINATION})

godot_add_library(godot-module-text-server-adv FOLDER modules STATIC
		${ICU_FONT_DATA_DESTINATION})
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-text-server-adv PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-text-server-adv PRIVATE
        register_types.cpp
        register_types.h
        script_iterator.cpp
        script_iterator.h
        text_server_adv.cpp
        text_server_adv.h
        thorvg_bounds_iterator.cpp
        thorvg_bounds_iterator.h
        thorvg_svg_in_ot.cpp
        thorvg_svg_in_ot.h
)
# [[[end]]]

target_link_libraries(godot-module-text-server-adv PUBLIC godot-object thorvg harfbuzz freetype msdfgen icu4c)
target_compile_definitions(godot-module-text-server-adv PRIVATE
		$<${WANT_EDITOR}:ICU_STATIC_DATA=1>
		-DICU_DATA_NAME=${ICU4C_DAT_NAME}
)
target_include_directories(godot-module-text-server-adv PRIVATE ${CMAKE_BINARY_INCLUDE_DIR}/thirdparty/icu4c)
