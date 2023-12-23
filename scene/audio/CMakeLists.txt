godot_add_library(godot-scene-audio FOLDER scene STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-scene-audio PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-scene-audio PRIVATE
        audio_stream_player.cpp
        audio_stream_player.h
        audio_stream_player_internal.cpp
        audio_stream_player_internal.h
)
# [[[end]]]
target_link_libraries(godot-scene-audio PUBLIC godot-templates)
