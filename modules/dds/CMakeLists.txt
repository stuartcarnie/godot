godot_add_library(godot-module-dds FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-dds PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-dds PRIVATE
        register_types.cpp
        register_types.h
        texture_loader_dds.cpp
        texture_loader_dds.h
)
# [[[end]]]
target_link_libraries(godot-module-dds PUBLIC godot-io)
