# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        audio_stream_editor_plugin.cpp
        audio_stream_editor_plugin.h
        audio_stream_preview.cpp
        audio_stream_preview.h
        audio_stream_randomizer_editor_plugin.cpp
        audio_stream_randomizer_editor_plugin.h
        editor_audio_buses.cpp
        editor_audio_buses.h
)
# [[[end]]]
