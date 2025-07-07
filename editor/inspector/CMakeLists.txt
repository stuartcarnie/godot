# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        add_metadata_dialog.cpp
        add_metadata_dialog.h
        editor_context_menu_plugin.cpp
        editor_context_menu_plugin.h
        editor_inspector.cpp
        editor_inspector.h
        editor_preview_plugins.cpp
        editor_preview_plugins.h
        editor_properties.cpp
        editor_properties.h
        editor_properties_array_dict.cpp
        editor_properties_array_dict.h
        editor_properties_vector.cpp
        editor_properties_vector.h
        editor_property_name_processor.cpp
        editor_property_name_processor.h
        editor_resource_picker.cpp
        editor_resource_picker.h
        editor_resource_preview.cpp
        editor_resource_preview.h
        editor_resource_tooltip_plugins.cpp
        editor_resource_tooltip_plugins.h
        editor_sectioned_inspector.cpp
        editor_sectioned_inspector.h
        input_event_editor_plugin.cpp
        input_event_editor_plugin.h
        multi_node_edit.cpp
        multi_node_edit.h
        property_selector.cpp
        property_selector.h
        sub_viewport_preview_editor_plugin.cpp
        sub_viewport_preview_editor_plugin.h
        tool_button_editor_plugin.cpp
        tool_button_editor_plugin.h
)
# [[[end]]]
