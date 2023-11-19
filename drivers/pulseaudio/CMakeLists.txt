if (${WANT_PULSEAUDIO} AND ${PULSEAUDIO_FOUND})
    godot_add_library(driver-pulseaudio FOLDER drivers STATIC)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(driver-pulseaudio PRIVATE')
    #   for file in list_files(cog, '.'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(driver-pulseaudio PRIVATE
            audio_driver_pulseaudio.cpp
            audio_driver_pulseaudio.h
            pulse-so_wrap.c
            pulse-so_wrap.h
    )
    # [[[end]]]
    target_link_libraries(driver-pulseaudio PUBLIC godot-os)
    target_compile_definitions(driver-pulseaudio PUBLIC PULSEAUDIO_ENABLED=1)
endif ()
