add_subdirectory(effects)
add_subdirectory(environment)
add_subdirectory(forward_clustered)
add_subdirectory(forward_mobile)
add_subdirectory(storage_rd)

godot_add_library(godot-server-rendering-renderer_rd FOLDER servers STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-server-rendering-renderer_rd PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-server-rendering-renderer_rd PRIVATE
        api_context_rd.cpp
        api_context_rd.h
        cluster_builder_rd.cpp
        cluster_builder_rd.h
        framebuffer_cache_rd.cpp
        framebuffer_cache_rd.h
        pipeline_cache_rd.cpp
        pipeline_cache_rd.h
        renderer_canvas_render_rd.cpp
        renderer_canvas_render_rd.h
        renderer_compositor_rd.cpp
        renderer_compositor_rd.h
        renderer_scene_render_rd.cpp
        renderer_scene_render_rd.h
        shader_rd.cpp
        shader_rd.h
        uniform_set_cache_rd.cpp
        uniform_set_cache_rd.h
)
# [[[end]]]
target_link_libraries(godot-server-rendering-renderer_rd PUBLIC godot-templates godot-os godot-glslang spirv-reflect)