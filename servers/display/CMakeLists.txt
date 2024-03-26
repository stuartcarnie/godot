godot_add_library(godot-server-display FOLDER servers STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-server-display PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-server-display PRIVATE
        native_menu.cpp
        native_menu.h
)
# [[[end]]]
target_link_libraries(godot-server-display PUBLIC godot-io)
