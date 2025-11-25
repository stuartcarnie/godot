# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        dock_constants.h
        editor_dock.cpp
        editor_dock.h
        editor_dock_manager.cpp
        editor_dock_manager.h
        filesystem_dock.cpp
        filesystem_dock.h
        groups_dock.cpp
        groups_dock.h
        groups_editor.cpp
        groups_editor.h
        history_dock.cpp
        history_dock.h
        import_dock.cpp
        import_dock.h
        inspector_dock.cpp
        inspector_dock.h
        scene_tree_dock.cpp
        scene_tree_dock.h
        signals_dock.cpp
        signals_dock.h
)
# [[[end]]]
