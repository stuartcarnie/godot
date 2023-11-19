godot_add_library(godot-module-bmp FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-bmp PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-bmp PRIVATE
        image_loader_bmp.cpp
        image_loader_bmp.h
        register_types.cpp
        register_types.h
)
# [[[end]]]
target_link_libraries(godot-module-bmp PUBLIC godot-io)
