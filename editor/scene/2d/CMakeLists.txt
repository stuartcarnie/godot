add_subdirectory(physics)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        abstract_polygon_2d_editor.cpp
        abstract_polygon_2d_editor.h
        camera_2d_editor_plugin.cpp
        camera_2d_editor_plugin.h
        light_occluder_2d_editor_plugin.cpp
        light_occluder_2d_editor_plugin.h
        line_2d_editor_plugin.cpp
        line_2d_editor_plugin.h
        parallax_background_editor_plugin.cpp
        parallax_background_editor_plugin.h
        particles_2d_editor_plugin.cpp
        particles_2d_editor_plugin.h
        path_2d_editor_plugin.cpp
        path_2d_editor_plugin.h
        polygon_2d_editor_plugin.cpp
        polygon_2d_editor_plugin.h
        scene_paint_2d_editor_plugin.cpp
        scene_paint_2d_editor_plugin.h
        skeleton_2d_editor_plugin.cpp
        skeleton_2d_editor_plugin.h
        sprite_2d_editor_plugin.cpp
        sprite_2d_editor_plugin.h
)
# [[[end]]]
