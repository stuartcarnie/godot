godot_add_library(godot-server-camera FOLDER servers STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-server-camera PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-server-camera PRIVATE
        camera_feed.cpp
        camera_feed.h
)
# [[[end]]]
target_link_libraries(godot-server-camera PUBLIC godot-io)
