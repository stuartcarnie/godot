godot_add_library(godot-editor-plugins-tiles FOLDER plugins)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-editor-plugins-tiles PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-editor-plugins-tiles PRIVATE
        atlas_merging_dialog.cpp
        atlas_merging_dialog.h
        tile_atlas_view.cpp
        tile_atlas_view.h
        tile_data_editors.cpp
        tile_data_editors.h
        tile_map_layer_editor.cpp
        tile_map_layer_editor.h
        tile_proxies_manager_dialog.cpp
        tile_proxies_manager_dialog.h
        tile_set_atlas_source_editor.cpp
        tile_set_atlas_source_editor.h
        tile_set_editor.cpp
        tile_set_editor.h
        tile_set_scenes_collection_source_editor.cpp
        tile_set_scenes_collection_source_editor.h
        tiles_editor_plugin.cpp
        tiles_editor_plugin.h
)
# [[[end]]]
target_link_libraries(godot-editor-plugins-tiles PRIVATE godot-config)
