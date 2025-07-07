# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        editor_dock_manager.cpp
        editor_dock_manager.h
        filesystem_dock.cpp
        filesystem_dock.h
        groups_editor.cpp
        groups_editor.h
        history_dock.cpp
        history_dock.h
        import_dock.cpp
        import_dock.h
        inspector_dock.cpp
        inspector_dock.h
        node_dock.cpp
        node_dock.h
        scene_tree_dock.cpp
        scene_tree_dock.h
)
# [[[end]]]
