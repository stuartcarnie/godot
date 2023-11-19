godot_add_library(godot-module-hdr FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-hdr PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-hdr PRIVATE
        image_loader_hdr.cpp
        image_loader_hdr.h
        register_types.cpp
        register_types.h
)
# [[[end]]]
target_link_libraries(godot-module-hdr PUBLIC godot-object)
