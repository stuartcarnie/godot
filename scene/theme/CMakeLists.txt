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
        icons/bookmark.svg
        icons/breakpoint.svg
        icons/checked.svg
        icons/checked_disabled.svg
        icons/close.svg
        icons/close_hl.svg
        icons/color_picker_bar_arrow.svg
        icons/color_picker_cursor.svg
        icons/color_picker_overbright.svg
        icons/color_picker_pipette.svg
        icons/error_icon.svg
        icons/file.svg
        icons/folder.svg
        icons/folder_create.svg
        icons/folder_up.svg
        icons/graph_port.svg
        icons/grid_layout.svg
        icons/grid_minimap.svg
        icons/grid_snap.svg
        icons/grid_toggle.svg
        icons/hslider_tick.svg
        icons/hsplitter.svg
        icons/indeterminate.svg
        icons/indeterminate_disabled.svg
        icons/line_edit_clear.svg
        icons/mini_checkerboard.svg
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
        icons/scroll_button_left.svg
        icons/scroll_button_left_hl.svg
        icons/scroll_button_right.svg
        icons/scroll_button_right_hl.svg
        icons/slider_grabber.svg
        icons/slider_grabber_disabled.svg
        icons/slider_grabber_hl.svg
        icons/tabs_drop_mark.svg
        icons/tabs_menu.svg
        icons/tabs_menu_hl.svg
        icons/text_edit_ellipsis.svg
        icons/text_edit_space.svg
        icons/text_edit_tab.svg
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
        icons/visibility_visible.svg
        icons/vslider_tick.svg
        icons/vsplitter.svg
        icons/zoom_less.svg
        icons/zoom_more.svg
        icons/zoom_reset.svg
)
# [[[end]]]

set(RESOURCES_SCENE_DEFAULT_GENERATED_ICONS ${CMAKE_BINARY_INCLUDE_DIR}/scene/resources/default_theme/default_theme_icons.gen.h)
generate_file(resource_make_default_theme_icons "${RESOURCE_DEFAULT_THEME_SOURCE}" "${RESOURCES_SCENE_DEFAULT_GENERATED_ICONS}")

godot_add_library(godot-scene-theme FOLDER scene STATIC ${RESOURCES_SCENE_DEFAULT_GENERATED_ICONS})
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-scene-theme PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-scene-theme PRIVATE
        default_theme.cpp
        default_theme.h
        theme_db.cpp
        theme_db.h
        theme_owner.cpp
        theme_owner.h
)
# [[[end]]]

target_link_libraries(godot-scene-theme PUBLIC godot-object godot-scene-resources)
target_include_directories(godot-scene-theme PRIVATE ${CMAKE_BINARY_INCLUDE_DIR}/scene/resources/default_theme)
