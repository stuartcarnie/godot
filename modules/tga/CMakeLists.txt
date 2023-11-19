godot_add_library(godot-module-tga FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-tga PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-tga PRIVATE
        image_loader_tga.cpp
        image_loader_tga.h
        register_types.cpp
        register_types.h
)
# [[[end]]]
target_link_libraries(godot-module-tga PUBLIC godot-object)
