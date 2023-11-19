godot_add_library(godot-server-debugger FOLDER servers STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-server-debugger PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-server-debugger PRIVATE
        servers_debugger.cpp
        servers_debugger.h
)
# [[[end]]]
target_link_libraries(godot-server-debugger PUBLIC godot-debugger)
