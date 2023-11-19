godot_add_library(godot-module-squish FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-squish PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-squish PRIVATE
        image_decompress_squish.cpp
        image_decompress_squish.h
        register_types.cpp
        register_types.h
)
# [[[end]]]
target_link_libraries(godot-module-squish PUBLIC godot-object squish)
