godot_add_library(godot-module-tilemap FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-tilemap PRIVATE')
#   for file in list_files(cog, ['.', '2d']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-tilemap PRIVATE
        register_types.cpp
        register_types.h
        tile_map.cpp
        tile_map.h
        tile_map_layer.cpp
        tile_map_layer.h
        tile_set.cpp
        tile_set.h
)
# [[[end]]]

if (WANT_EDITOR)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(godot-module-tilemap PRIVATE')
    #   for file in list_files(cog, 'editor'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(godot-module-tilemap PRIVATE
            editor/atlas_merging_dialog.cpp
            editor/atlas_merging_dialog.h
            editor/tile_atlas_view.cpp
            editor/tile_atlas_view.h
            editor/tile_data_editors.cpp
            editor/tile_data_editors.h
            editor/tile_map_layer_editor.cpp
            editor/tile_map_layer_editor.h
            editor/tile_proxies_manager_dialog.cpp
            editor/tile_proxies_manager_dialog.h
            editor/tile_set_atlas_source_editor.cpp
            editor/tile_set_atlas_source_editor.h
            editor/tile_set_editor.cpp
            editor/tile_set_editor.h
            editor/tile_set_scenes_collection_source_editor.cpp
            editor/tile_set_scenes_collection_source_editor.h
            editor/tiles_editor_plugin.cpp
            editor/tiles_editor_plugin.h
    )
    # [[[end]]]
endif ()

target_link_libraries(godot-module-tilemap PUBLIC core)
group_target_sources(godot-module-tilemap)
