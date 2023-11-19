cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

if (${WANT_ALSA} AND ${ALSA_FOUND})
    godot_add_library(driver-alsa FOLDER drivers STATIC)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(driver-alsa PRIVATE')
    #   for file in list_files(cog, '.'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(driver-alsa PRIVATE
            asound-so_wrap.c
            asound-so_wrap.h
            audio_driver_alsa.cpp
            audio_driver_alsa.h
    )
    # [[[end]]]

    target_compile_definitions(driver-alsa PUBLIC ALSA_ENABLED=1 PULSEAUDIO_ENABLED=1)
    target_link_libraries(driver-alsa PUBLIC linuxbsd-headers ${ALSA_LIBRARIES} godot-error)

    #Inform that pulse audio is available
    if (${WANT_PULSEAUDIO} AND ${PULSEAUDIO_FOUND})
        message(STATUS, "Found PulseAudio library")
        target_compile_definitions(driver-alsa PRIVATE "-DPULSEAUDIO_ENABLED=1")
    else ()
        message(STATUS, "Found PulseAudio missing.... disabling")
    endif ()
endif ()