godot_add_library(godot-editor-import-3d FOLDER import/3d)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-editor-import-3d PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-editor-import-3d PRIVATE
        collada.cpp
        collada.h
        editor_import_collada.cpp
        editor_import_collada.h
        post_import_plugin_skeleton_renamer.cpp
        post_import_plugin_skeleton_renamer.h
        post_import_plugin_skeleton_rest_fixer.cpp
        post_import_plugin_skeleton_rest_fixer.h
        post_import_plugin_skeleton_track_organizer.cpp
        post_import_plugin_skeleton_track_organizer.h
        resource_importer_obj.cpp
        resource_importer_obj.h
        resource_importer_scene.cpp
        resource_importer_scene.h
        scene_import_settings.cpp
        scene_import_settings.h
)
# [[[end]]]
target_link_libraries(godot-editor-import-3d PRIVATE godot-config)
