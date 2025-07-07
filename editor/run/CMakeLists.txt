# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        editor_run.cpp
        editor_run.h
        editor_run_bar.cpp
        editor_run_bar.h
        editor_run_native.cpp
        editor_run_native.h
        embedded_process.cpp
        embedded_process.h
        game_view_plugin.cpp
        game_view_plugin.h
        run_instances_dialog.cpp
        run_instances_dialog.h
)
# [[[end]]]
