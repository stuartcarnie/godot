# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(servers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(servers PRIVATE
        camera_attributes_storage.cpp
        camera_attributes_storage.h
        compositor_storage.cpp
        compositor_storage.h
        environment_storage.cpp
        environment_storage.h
        light_storage.h
        material_storage.h
        mesh_storage.cpp
        mesh_storage.h
        particles_storage.h
        render_data.cpp
        render_data.h
        render_scene_buffers.cpp
        render_scene_buffers.h
        render_scene_data.cpp
        render_scene_data.h
        texture_storage.h
        utilities.cpp
        utilities.h
        variant_converters.h
)
# [[[end]]]
