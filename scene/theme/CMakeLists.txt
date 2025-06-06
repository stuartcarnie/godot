# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('set_abs(RESOURCE_DEFAULT_THEME_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}')
#   for file in list_files(cog, 'icons', exts=['svg']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
set_abs(RESOURCE_DEFAULT_THEME_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}
        icons/add.svg
        icons/arrow_down.svg
        icons/arrow_left.svg
        icons/arrow_right.svg
        icons/arrow_up.svg
        icons/bookmark.svg
        icons/breakpoint.svg
        icons/checked.svg
        icons/checked_disabled.svg
        icons/clear.svg
        icons/close.svg
        icons/close_hl.svg
        icons/color_picker_bar_arrow.svg
        icons/color_picker_cursor.svg
        icons/color_picker_cursor_bg.svg
        icons/color_picker_overbright.svg
        icons/color_picker_pipette.svg
        icons/error_icon.svg
        icons/favorite.svg
        icons/file.svg
        icons/file_mode_list.svg
        icons/file_mode_thumbnail.svg
        icons/file_thumbnail.svg
        icons/folder.svg
        icons/folder_create.svg
        icons/folder_thumbnail.svg
        icons/folder_up.svg
        icons/graph_port.svg
        icons/grid_layout.svg
        icons/grid_minimap.svg
        icons/grid_snap.svg
        icons/grid_toggle.svg
        icons/h_dragger.svg
        icons/hslider_tick.svg
        icons/hsplitter.svg
        icons/indeterminate.svg
        icons/indeterminate_disabled.svg
        icons/line_edit_clear.svg
        icons/load.svg
        icons/mini_checkerboard.svg
        icons/move_down.svg
        icons/move_up.svg
        icons/option_button_arrow.svg
        icons/picker_shape_circle.svg
        icons/picker_shape_rectangle.svg
        icons/picker_shape_rectangle_wheel.svg
        icons/popup_menu_arrow_left.svg
        icons/popup_menu_arrow_right.svg
        icons/radio_checked.svg
        icons/radio_checked_disabled.svg
        icons/radio_unchecked.svg
        icons/radio_unchecked_disabled.svg
        icons/region_folded.svg
        icons/region_unfolded.svg
        icons/reload.svg
        icons/resizer_nw.svg
        icons/resizer_se.svg
        icons/save.svg
        icons/script.svg
        icons/scroll_button_left.svg
        icons/scroll_button_left_hl.svg
        icons/scroll_button_right.svg
        icons/scroll_button_right_hl.svg
        icons/slider_grabber.svg
        icons/slider_grabber_disabled.svg
        icons/slider_grabber_hl.svg
        icons/sort.svg
        icons/tabs_drop_mark.svg
        icons/tabs_menu.svg
        icons/tabs_menu_hl.svg
        icons/text_edit_ellipsis.svg
        icons/text_edit_space.svg
        icons/text_edit_tab.svg
        icons/toggle_filename_filter.svg
        icons/toggle_off.svg
        icons/toggle_off_disabled.svg
        icons/toggle_off_disabled_mirrored.svg
        icons/toggle_off_mirrored.svg
        icons/toggle_on.svg
        icons/toggle_on_disabled.svg
        icons/toggle_on_disabled_mirrored.svg
        icons/toggle_on_mirrored.svg
        icons/unchecked.svg
        icons/unchecked_disabled.svg
        icons/updown.svg
        icons/v_dragger.svg
        icons/value_down.svg
        icons/value_up.svg
        icons/visibility_visible.svg
        icons/vslider_tick.svg
        icons/vsplitter.svg
        icons/zoom_less.svg
        icons/zoom_more.svg
        icons/zoom_reset.svg
)
# [[[end]]]

set(RESOURCES_SCENE_DEFAULT_GENERATED_ICONS ${CMAKE_BINARY_INCLUDE_DIR}/scene/theme/default_theme_icons.gen.h)
generate_file(resource_make_default_theme_icons "${RESOURCE_DEFAULT_THEME_SOURCE}" "${RESOURCES_SCENE_DEFAULT_GENERATED_ICONS}")

add_custom_target(scene-icons-generated
        DEPENDS
        ${RESOURCES_SCENE_DEFAULT_GENERATED_ICONS}
)
set_target_properties(scene-icons-generated PROPERTIES FOLDER "generated")

set(RESOURCES_DEFAULT_FONT_SOURCE ${GODOT_SOURCE_ROOT_THIRDPARTY_DIR}/fonts/OpenSans_SemiBold.woff2)
set(RESOURCES_DEFAULT_FONT_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/scene/theme/default_font.gen.h)
generate_file(resource_scene_make_fonts_header ${RESOURCES_DEFAULT_FONT_SOURCE} ${RESOURCES_DEFAULT_FONT_OUTPUT})

add_custom_target(scene-fonts-generated
        DEPENDS
        ${RESOURCES_DEFAULT_FONT_OUTPUT})
set_target_properties(scene-fonts-generated PROPERTIES FOLDER "generated")

add_dependencies(scene scene-icons-generated scene-fonts-generated)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(scene PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(scene PRIVATE
        default_theme.cpp
        default_theme.h
        theme_db.cpp
        theme_db.h
        theme_owner.cpp
        theme_owner.h
)
# [[[end]]]

target_compile_definitions(scene
        PRIVATE
        $<$<BOOL:${ENABLE_BROTLI}>:BROTLI_ENABLED>
)
