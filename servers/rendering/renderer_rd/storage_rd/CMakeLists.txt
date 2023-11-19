godot_add_library(godot-server-rendering-renderer_rd-storage_rd FOLDER rendering STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-server-rendering-renderer_rd-storage_rd PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-server-rendering-renderer_rd-storage_rd PRIVATE
        forward_id_storage.cpp
        forward_id_storage.h
        light_storage.cpp
        light_storage.h
        material_storage.cpp
        material_storage.h
        mesh_storage.cpp
        mesh_storage.h
        particles_storage.cpp
        particles_storage.h
        render_buffer_custom_data_rd.h
        render_scene_buffers_rd.cpp
        render_scene_buffers_rd.h
        render_scene_data_rd.cpp
        render_scene_data_rd.h
        texture_storage.cpp
        texture_storage.h
        utilities.cpp
        utilities.h
)
# [[[end]]]

target_link_libraries(godot-server-rendering-renderer_rd-storage_rd PRIVATE godot-config)
