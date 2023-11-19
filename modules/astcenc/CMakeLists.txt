godot_add_library(godot-module-astcenc FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-astcenc PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-astcenc PRIVATE
        image_compress_astcenc.cpp
        image_compress_astcenc.h
        register_types.cpp
        register_types.h
)
# [[[end]]]
target_link_libraries(godot-module-astcenc PUBLIC godot-io astcenc)
