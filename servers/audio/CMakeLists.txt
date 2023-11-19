add_subdirectory(effects)

godot_add_library(godot-server-audio FOLDER servers STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-server-audio PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-server-audio PRIVATE
        audio_driver_dummy.cpp
        audio_driver_dummy.h
        audio_effect.cpp
        audio_effect.h
        audio_filter_sw.cpp
        audio_filter_sw.h
        audio_rb_resampler.cpp
        audio_rb_resampler.h
        audio_stream.cpp
        audio_stream.h
)
# [[[end]]]
target_link_libraries(godot-server-audio PRIVATE godot-config)
