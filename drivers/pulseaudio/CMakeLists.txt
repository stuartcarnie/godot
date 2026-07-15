if (${WANT_PULSEAUDIO} AND ${PULSEAUDIO_FOUND})
    godot_add_library(pulseaudio FOLDER drivers STATIC)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(pulseaudio PRIVATE')
    #   for file in list_files(cog, '.'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(pulseaudio PRIVATE
            audio_driver_pulseaudio.cpp
            audio_driver_pulseaudio.h
            pulse-so_wrap.c
            pulse-so_wrap.h
    )
    # [[[end]]]
    target_link_libraries(pulseaudio PUBLIC core)
    target_compile_definitions(pulseaudio PUBLIC PULSEAUDIO_ENABLED)

    group_target_sources(pulseaudio)
endif ()
