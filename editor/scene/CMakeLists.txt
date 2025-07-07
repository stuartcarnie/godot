add_subdirectory(2d)
add_subdirectory(3d)
add_subdirectory(gui)
add_subdirectory(texture)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        bone_map_editor_plugin.cpp
        bone_map_editor_plugin.h
        canvas_item_editor_plugin.cpp
        canvas_item_editor_plugin.h
        connections_dialog.cpp
        connections_dialog.h
        curve_editor_plugin.cpp
        curve_editor_plugin.h
        editor_scene_tabs.cpp
        editor_scene_tabs.h
        gpu_particles_collision_sdf_editor_plugin.cpp
        gpu_particles_collision_sdf_editor_plugin.h
        gradient_editor_plugin.cpp
        gradient_editor_plugin.h
        group_settings_editor.cpp
        group_settings_editor.h
        material_editor_plugin.cpp
        material_editor_plugin.h
        packed_scene_editor_plugin.cpp
        packed_scene_editor_plugin.h
        particle_process_material_editor_plugin.cpp
        particle_process_material_editor_plugin.h
        particles_editor_plugin.cpp
        particles_editor_plugin.h
        rename_dialog.cpp
        rename_dialog.h
        reparent_dialog.cpp
        reparent_dialog.h
        resource_preloader_editor_plugin.cpp
        resource_preloader_editor_plugin.h
        scene_create_dialog.cpp
        scene_create_dialog.h
        scene_tree_editor.cpp
        scene_tree_editor.h
        sprite_frames_editor_plugin.cpp
        sprite_frames_editor_plugin.h
)
# [[[end]]]
