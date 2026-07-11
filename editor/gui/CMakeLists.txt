# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        code_editor.cpp
        code_editor.h
        create_dialog.cpp
        create_dialog.h
        credits_roll.cpp
        credits_roll.h
        directory_create_dialog.cpp
        directory_create_dialog.h
        editor_about.cpp
        editor_about.h
        editor_bottom_panel.cpp
        editor_bottom_panel.h
        editor_dir_dialog.cpp
        editor_dir_dialog.h
        editor_file_dialog.cpp
        editor_file_dialog.h
        editor_icon_manager.cpp
        editor_icon_manager.h
        editor_object_selector.cpp
        editor_object_selector.h
        editor_quick_open_dialog.cpp
        editor_quick_open_dialog.h
        editor_spin_slider.cpp
        editor_spin_slider.h
        editor_title_bar.cpp
        editor_title_bar.h
        editor_toaster.cpp
        editor_toaster.h
        editor_validation_panel.cpp
        editor_validation_panel.h
        editor_variant_type_selectors.cpp
        editor_variant_type_selectors.h
        editor_version_button.cpp
        editor_version_button.h
        editor_zoom_widget.cpp
        editor_zoom_widget.h
        filter_line_edit.cpp
        filter_line_edit.h
        progress_dialog.cpp
        progress_dialog.h
        touch_actions_panel.cpp
        touch_actions_panel.h
        window_wrapper.cpp
        window_wrapper.h
)
# [[[end]]]
