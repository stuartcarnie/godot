godot_add_library(godot-server-rendering-storage FOLDER servers STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-server-rendering-storage PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-server-rendering-storage PRIVATE
        camera_attributes_storage.cpp
        camera_attributes_storage.h
        environment_storage.cpp
        environment_storage.h
        light_storage.h
        material_storage.h
        mesh_storage.h
        particles_storage.h
        render_scene_buffers.cpp
        render_scene_buffers.h
        texture_storage.h
        utilities.cpp
        utilities.h
        variant_converters.h
)
# [[[end]]]

target_link_libraries(godot-server-rendering-storage PRIVATE godot-os)