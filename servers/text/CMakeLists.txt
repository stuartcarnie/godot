godot_add_library(godot-server-text FOLDER servers STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-server-text PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-server-text PRIVATE
        text_server_dummy.h
        text_server_extension.cpp
        text_server_extension.h
)
# [[[end]]]
target_link_libraries(godot-server-text PUBLIC godot-object)
