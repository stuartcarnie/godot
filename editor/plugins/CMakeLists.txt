# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        editor_plugin.cpp
        editor_plugin.h
        editor_plugin_list.cpp
        editor_plugin_list.h
        editor_plugin_settings.cpp
        editor_plugin_settings.h
        editor_resource_conversion_plugin.cpp
        editor_resource_conversion_plugin.h
        plugin_config_dialog.cpp
        plugin_config_dialog.h
)
# [[[end]]]
