# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        editor_script.cpp
        editor_script.h
        editor_script_plugin.cpp
        editor_script_plugin.h
        find_in_files.cpp
        find_in_files.h
        script_create_dialog.cpp
        script_create_dialog.h
        script_editor_plugin.cpp
        script_editor_plugin.h
        script_text_editor.cpp
        script_text_editor.h
        text_editor.cpp
        text_editor.h
)
# [[[end]]]
