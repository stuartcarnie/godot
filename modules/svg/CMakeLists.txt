godot_add_library(godot-module-svg FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-svg PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-svg PRIVATE
        image_loader_svg.cpp
        image_loader_svg.h
        register_types.cpp
        register_types.h
)
# [[[end]]]
target_link_libraries(godot-module-svg PUBLIC godot-object thorvg)
