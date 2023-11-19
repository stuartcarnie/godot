godot_add_library(godot-server-extensions FOLDER servers STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-server-extensions PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-server-extensions PRIVATE
        physics_server_2d_extension.cpp
        physics_server_2d_extension.h
        physics_server_3d_extension.cpp
        physics_server_3d_extension.h
)
# [[[end]]]
target_link_libraries(godot-server-extensions PUBLIC godot-extension)
