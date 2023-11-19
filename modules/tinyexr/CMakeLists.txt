godot_add_library(godot-module-tinyexr FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-tinyexr PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-tinyexr PRIVATE
        image_loader_tinyexr.cpp
        image_loader_tinyexr.h
        image_saver_tinyexr.cpp
        image_saver_tinyexr.h
        register_types.cpp
        register_types.h
)
# [[[end]]]
target_link_libraries(godot-module-tinyexr PUBLIC godot-object tinyexr zlib)
