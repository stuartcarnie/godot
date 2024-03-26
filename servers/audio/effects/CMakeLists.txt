godot_add_library(godot-server-audio-effects FOLDER audio STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-server-audio-effects PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-server-audio-effects PRIVATE
        audio_effect_amplify.cpp
        audio_effect_amplify.h
        audio_effect_capture.cpp
        audio_effect_capture.h
        audio_effect_chorus.cpp
        audio_effect_chorus.h
        audio_effect_compressor.cpp
        audio_effect_compressor.h
        audio_effect_delay.cpp
        audio_effect_delay.h
        audio_effect_distortion.cpp
        audio_effect_distortion.h
        audio_effect_eq.cpp
        audio_effect_eq.h
        audio_effect_filter.cpp
        audio_effect_filter.h
        audio_effect_hard_limiter.cpp
        audio_effect_hard_limiter.h
        audio_effect_limiter.cpp
        audio_effect_limiter.h
        audio_effect_panner.cpp
        audio_effect_panner.h
        audio_effect_phaser.cpp
        audio_effect_phaser.h
        audio_effect_pitch_shift.cpp
        audio_effect_pitch_shift.h
        audio_effect_record.cpp
        audio_effect_record.h
        audio_effect_reverb.cpp
        audio_effect_reverb.h
        audio_effect_spectrum_analyzer.cpp
        audio_effect_spectrum_analyzer.h
        audio_effect_stereo_enhance.cpp
        audio_effect_stereo_enhance.h
        audio_stream_generator.cpp
        audio_stream_generator.h
        eq_filter.cpp
        eq_filter.h
        reverb_filter.cpp
        reverb_filter.h
)
# [[[end]]]
target_link_libraries(godot-server-audio-effects PRIVATE godot-config)
