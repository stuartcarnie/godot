add_subdirectory(effects)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(servers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(servers PRIVATE
        audio_driver_dummy.cpp
        audio_driver_dummy.h
        audio_effect.cpp
        audio_effect.h
        audio_filter_sw.cpp
        audio_filter_sw.h
        audio_rb_resampler.cpp
        audio_rb_resampler.h
        audio_server.cpp
        audio_server.h
        audio_stream.cpp
        audio_stream.h
)
# [[[end]]]
