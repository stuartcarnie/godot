# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        cast_2d_editor_plugin.cpp
        cast_2d_editor_plugin.h
        collision_polygon_2d_editor_plugin.cpp
        collision_polygon_2d_editor_plugin.h
        collision_shape_2d_editor_plugin.cpp
        collision_shape_2d_editor_plugin.h
)
# [[[end]]]
