godot_add_library(godot-server-rendering-dummy FOLDER servers STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-server-rendering-dummy PRIVATE')
#   for file in list_files(cog, ['environment', '.', 'storage']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-server-rendering-dummy PRIVATE
        environment/fog.h
        environment/gi.h
        rasterizer_canvas_dummy.h
        rasterizer_dummy.h
        rasterizer_scene_dummy.h
        storage/light_storage.h
        storage/material_storage.h
        storage/mesh_storage.cpp
        storage/mesh_storage.h
        storage/particles_storage.h
        storage/texture_storage.cpp
        storage/texture_storage.h
        storage/utilities.cpp
        storage/utilities.h
)
# [[[end]]]
target_link_libraries(godot-server-rendering-dummy PRIVATE godot-os)