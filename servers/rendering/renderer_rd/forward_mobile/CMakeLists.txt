godot_add_library(godot-server-rendering-renderer_rd-forward_mobile FOLDER rendering STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-server-rendering-renderer_rd-forward_mobile PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-server-rendering-renderer_rd-forward_mobile PRIVATE
        render_forward_mobile.cpp
        render_forward_mobile.h
        scene_shader_forward_mobile.cpp
        scene_shader_forward_mobile.h
)
# [[[end]]]

target_link_libraries(godot-server-rendering-renderer_rd-forward_mobile PRIVATE godot-config)
