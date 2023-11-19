godot_add_library(godot-module-raycast FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-raycast PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-raycast PRIVATE
        lightmap_raycaster_embree.cpp
        lightmap_raycaster_embree.h
        raycast_occlusion_cull.cpp
        raycast_occlusion_cull.h
        register_types.cpp
        register_types.h
        static_raycaster_embree.cpp
        static_raycaster_embree.h
)
# [[[end]]]
target_link_libraries(godot-module-raycast PUBLIC godot-object embree)
