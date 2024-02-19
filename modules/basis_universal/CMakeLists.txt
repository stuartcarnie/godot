godot_add_library(godot-module-basis-universal FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-basis-universal PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-basis-universal PRIVATE
        image_compress_basisu.cpp
        image_compress_basisu.h
        register_types.cpp
        register_types.h
)
# [[[end]]]
target_link_libraries(godot-module-basis-universal PUBLIC godot-object basis_universal)
