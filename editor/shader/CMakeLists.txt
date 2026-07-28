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
        editor_shader_language_plugin.cpp
        editor_shader_language_plugin.h
        shader_create_dialog.cpp
        shader_create_dialog.h
        shader_editor_plugin.cpp
        shader_editor_plugin.h
        shader_file_editor_plugin.cpp
        shader_file_editor_plugin.h
        shader_globals_editor.cpp
        shader_globals_editor.h
        shader_text_editor.cpp
        shader_text_editor.h
        text_shader_language_plugin.cpp
        text_shader_language_plugin.h
)
# [[[end]]]
