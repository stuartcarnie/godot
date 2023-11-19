godot_add_library(godot-module-cvtt FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-cvtt PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-cvtt PRIVATE
        image_compress_cvtt.cpp
        image_compress_cvtt.h
        register_types.cpp
        register_types.h
)
# [[[end]]]

target_link_libraries(godot-module-cvtt PUBLIC godot-io cvtt)
