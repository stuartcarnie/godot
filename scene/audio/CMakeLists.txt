# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(scene PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(scene PRIVATE
        audio_stream_player.cpp
        audio_stream_player.h
        audio_stream_player_internal.cpp
        audio_stream_player_internal.h
)
# [[[end]]]
