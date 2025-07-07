# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        control_editor_plugin.cpp
        control_editor_plugin.h
        font_config_plugin.cpp
        font_config_plugin.h
        style_box_editor_plugin.cpp
        style_box_editor_plugin.h
        theme_editor_plugin.cpp
        theme_editor_plugin.h
        theme_editor_preview.cpp
        theme_editor_preview.h
)
# [[[end]]]
