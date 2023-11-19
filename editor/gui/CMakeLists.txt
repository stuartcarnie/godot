godot_add_library(godot-editor-gui FOLDER gui)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-editor-gui PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-editor-gui PRIVATE
        editor_dir_dialog.cpp
        editor_dir_dialog.h
        editor_file_dialog.cpp
        editor_file_dialog.h
        editor_object_selector.cpp
        editor_object_selector.h
        editor_run_bar.cpp
        editor_run_bar.h
        editor_scene_tabs.cpp
        editor_scene_tabs.h
        editor_spin_slider.cpp
        editor_spin_slider.h
        editor_title_bar.cpp
        editor_title_bar.h
        editor_toaster.cpp
        editor_toaster.h
        editor_validation_panel.cpp
        editor_validation_panel.h
        editor_zoom_widget.cpp
        editor_zoom_widget.h
        scene_tree_editor.cpp
        scene_tree_editor.h
)
# [[[end]]]
target_link_libraries(godot-editor-gui PRIVATE godot-config)
