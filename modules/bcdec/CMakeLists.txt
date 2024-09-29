godot_add_library(godot-module-bcdec FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-bcdec PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-bcdec PRIVATE
        image_decompress_bcdec.cpp
        image_decompress_bcdec.h
        register_types.cpp
        register_types.h
)
# [[[end]]]
target_link_libraries(godot-module-bcdec PUBLIC godot-object godot-thirdparty-misc)
