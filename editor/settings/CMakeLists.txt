# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        action_map_editor.cpp
        action_map_editor.h
        editor_autoload_settings.cpp
        editor_autoload_settings.h
        editor_build_profile.cpp
        editor_build_profile.h
        editor_command_palette.cpp
        editor_command_palette.h
        editor_event_search_bar.cpp
        editor_event_search_bar.h
        editor_feature_profile.cpp
        editor_feature_profile.h
        editor_folding.cpp
        editor_folding.h
        editor_layouts_dialog.cpp
        editor_layouts_dialog.h
        editor_settings.cpp
        editor_settings.h
        editor_settings_dialog.cpp
        editor_settings_dialog.h
        event_listener_line_edit.cpp
        event_listener_line_edit.h
        input_event_configuration_dialog.cpp
        input_event_configuration_dialog.h
        project_settings_editor.cpp
        project_settings_editor.h
)
# [[[end]]]
