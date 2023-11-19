godot_add_library(godot-server-rendering-renderer_rd-environment FOLDER rendering STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-server-rendering-renderer_rd-environment PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-server-rendering-renderer_rd-environment PRIVATE
        fog.cpp
        fog.h
        gi.cpp
        gi.h
        sky.cpp
        sky.h
)
# [[[end]]]

target_link_libraries(godot-server-rendering-renderer_rd-environment
        PRIVATE godot-config
)
