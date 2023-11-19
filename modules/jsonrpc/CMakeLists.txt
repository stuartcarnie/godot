godot_add_library(godot-module-jsonrpc FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-jsonrpc PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-jsonrpc PRIVATE
        jsonrpc.cpp
        jsonrpc.h
        register_types.cpp
        register_types.h
)
# [[[end]]]
target_link_libraries(godot-module-jsonrpc PUBLIC godot-object)
