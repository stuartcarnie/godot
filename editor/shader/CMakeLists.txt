add_subdirectory(shader_baker)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        editor_native_shader_source_visualizer.cpp
        editor_native_shader_source_visualizer.h
        shader_create_dialog.cpp
        shader_create_dialog.h
        shader_editor.h
        shader_editor_plugin.cpp
        shader_editor_plugin.h
        shader_file_editor_plugin.cpp
        shader_file_editor_plugin.h
        shader_globals_editor.cpp
        shader_globals_editor.h
        text_shader_editor.cpp
        text_shader_editor.h
        visual_shader_editor_plugin.cpp
        visual_shader_editor_plugin.h
)
# [[[end]]]
