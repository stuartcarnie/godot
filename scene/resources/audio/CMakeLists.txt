# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(scene PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(scene PRIVATE
        audio_stream.cpp
        audio_stream.h
        audio_stream_generator.cpp
        audio_stream_generator.h
        audio_stream_microphone.cpp
        audio_stream_microphone.h
        audio_stream_polyphonic.cpp
        audio_stream_polyphonic.h
        audio_stream_randomizer.cpp
        audio_stream_randomizer.h
        audio_stream_wav.cpp
        audio_stream_wav.h
)
# [[[end]]]
