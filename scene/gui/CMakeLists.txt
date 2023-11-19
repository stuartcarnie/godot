godot_add_library(godot-scene-gui FOLDER scene STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-scene-gui PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-scene-gui PRIVATE
        aspect_ratio_container.cpp
        aspect_ratio_container.h
        base_button.cpp
        base_button.h
        box_container.cpp
        box_container.h
        button.cpp
        button.h
        center_container.cpp
        center_container.h
        check_box.cpp
        check_box.h
        check_button.cpp
        check_button.h
        code_edit.cpp
        code_edit.h
        color_mode.cpp
        color_mode.h
        color_picker.cpp
        color_picker.h
        color_rect.cpp
        color_rect.h
        container.cpp
        container.h
        control.cpp
        control.h
        dialogs.cpp
        dialogs.h
        file_dialog.cpp
        file_dialog.h
        flow_container.cpp
        flow_container.h
        graph_edit.cpp
        graph_edit.h
        graph_edit_arranger.cpp
        graph_edit_arranger.h
        graph_element.cpp
        graph_element.h
        graph_node.cpp
        graph_node.h
        grid_container.cpp
        grid_container.h
        item_list.cpp
        item_list.h
        label.cpp
        label.h
        line_edit.cpp
        line_edit.h
        link_button.cpp
        link_button.h
        margin_container.cpp
        margin_container.h
        menu_bar.cpp
        menu_bar.h
        menu_button.cpp
        menu_button.h
        nine_patch_rect.cpp
        nine_patch_rect.h
        option_button.cpp
        option_button.h
        panel.cpp
        panel.h
        panel_container.cpp
        panel_container.h
        popup.cpp
        popup.h
        popup_menu.cpp
        popup_menu.h
        progress_bar.cpp
        progress_bar.h
        range.cpp
        range.h
        reference_rect.cpp
        reference_rect.h
        rich_text_effect.cpp
        rich_text_effect.h
        rich_text_label.cpp
        rich_text_label.h
        scroll_bar.cpp
        scroll_bar.h
        scroll_container.cpp
        scroll_container.h
        separator.cpp
        separator.h
        slider.cpp
        slider.h
        spin_box.cpp
        spin_box.h
        split_container.cpp
        split_container.h
        subviewport_container.cpp
        subviewport_container.h
        tab_bar.cpp
        tab_bar.h
        tab_container.cpp
        tab_container.h
        text_edit.cpp
        text_edit.h
        texture_button.cpp
        texture_button.h
        texture_progress_bar.cpp
        texture_progress_bar.h
        texture_rect.cpp
        texture_rect.h
        tree.cpp
        tree.h
        video_stream_player.cpp
        video_stream_player.h
        view_panner.cpp
        view_panner.h
)
# [[[end]]]
target_link_libraries(godot-scene-gui PUBLIC godot-input godot-server-audio)
