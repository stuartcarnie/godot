if (${WANT_ALSA} AND ${ALSA_FOUND})
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(drivers PRIVATE')
    #   for file in list_files(cog, '.'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(drivers PRIVATE
            asound-so_wrap.c
            asound-so_wrap.h
            audio_driver_alsa.cpp
            audio_driver_alsa.h
    )
    # [[[end]]]

    target_compile_definitions(drivers PUBLIC ALSA_ENABLED=1 PULSEAUDIO_ENABLED=1)
    target_link_libraries(drivers PUBLIC linuxbsd-headers ${ALSA_LIBRARIES})

    #Inform that pulse audio is available
    if (${WANT_PULSEAUDIO} AND ${PULSEAUDIO_FOUND})
        message(STATUS, "Found PulseAudio library")
        target_compile_definitions(drivers PRIVATE "-DPULSEAUDIO_ENABLED=1")
    else ()
        message(STATUS, "Found PulseAudio missing.... disabling")
    endif ()
endif ()
