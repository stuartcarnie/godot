godot_add_library(godot-server-rendering-renderer_rd-forward_clustered FOLDER rendering STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-server-rendering-renderer_rd-forward_clustered PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-server-rendering-renderer_rd-forward_clustered PRIVATE
        render_forward_clustered.cpp
        render_forward_clustered.h
        scene_shader_forward_clustered.cpp
        scene_shader_forward_clustered.h
)
# [[[end]]]

target_link_libraries(godot-server-rendering-renderer_rd-forward_clustered PRIVATE godot-config)
