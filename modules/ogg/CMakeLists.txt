godot_add_library(godot-module-ogg FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-ogg PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-ogg PRIVATE
        ogg_packet_sequence.cpp
        ogg_packet_sequence.h
        register_types.cpp
        register_types.h
)
# [[[end]]]
target_link_libraries(godot-module-ogg PUBLIC godot-object ogg)
