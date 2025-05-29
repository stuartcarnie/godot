add_compile_definitions($<$<BOOL:${WANT_METAL}>:METAL_ENABLED=1>)

add_subdirectory(dummy)
add_subdirectory(renderer_rd)
add_subdirectory(storage)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(servers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(servers PRIVATE
        instance_uniforms.cpp
        instance_uniforms.h
        renderer_canvas_cull.cpp
        renderer_canvas_cull.h
        renderer_canvas_render.cpp
        renderer_canvas_render.h
        renderer_compositor.cpp
        renderer_compositor.h
        renderer_geometry_instance.cpp
        renderer_geometry_instance.h
        renderer_scene_cull.cpp
        renderer_scene_cull.h
        renderer_scene_occlusion_cull.cpp
        renderer_scene_occlusion_cull.h
        renderer_scene_render.cpp
        renderer_scene_render.h
        renderer_viewport.cpp
        renderer_viewport.h
        rendering_context_driver.cpp
        rendering_context_driver.h
        rendering_device.cpp
        rendering_device.h
        rendering_device_binds.cpp
        rendering_device_binds.h
        rendering_device_commons.cpp
        rendering_device_commons.h
        rendering_device_driver.cpp
        rendering_device_driver.h
        rendering_device_graph.cpp
        rendering_device_graph.h
        rendering_light_culler.cpp
        rendering_light_culler.h
        rendering_method.cpp
        rendering_method.h
        rendering_server_constants.h
        rendering_server_default.cpp
        rendering_server_default.h
        rendering_server_globals.cpp
        rendering_server_globals.h
        rendering_shader_container.cpp
        rendering_shader_container.h
        rendering_shader_library.h
        shader_compiler.cpp
        shader_compiler.h
        shader_include_db.cpp
        shader_include_db.h
        shader_language.cpp
        shader_language.h
        shader_preprocessor.cpp
        shader_preprocessor.h
        shader_types.cpp
        shader_types.h
        shader_warnings.cpp
        shader_warnings.h
)
# [[[end]]]
target_include_directories(servers PUBLIC ${CMAKE_BINARY_INCLUDE_DIR}) #Some other targets depend on specific generated shaders

