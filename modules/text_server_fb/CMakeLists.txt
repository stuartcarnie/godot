godot_add_library(godot-module-text-server-fb FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-text-server-fb PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-text-server-fb PRIVATE
        register_types.cpp
        register_types.h
        text_server_fb.cpp
        text_server_fb.h
        thorvg_bounds_iterator.cpp
        thorvg_bounds_iterator.h
        thorvg_svg_in_ot.cpp
        thorvg_svg_in_ot.h
)
# [[[end]]]
target_link_libraries(godot-module-text-server-fb PUBLIC godot-object thorvg freetype msdfgen)
