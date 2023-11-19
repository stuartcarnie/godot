if (${WANT_ALSA} AND ${ALSA_FOUND})
    godot_add_library(driver-alsa-midi FOLDER drivers STATIC)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(driver-alsa-midi PRIVATE')
    #   for file in list_files(cog, '.'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(driver-alsa-midi PRIVATE
            midi_driver_alsamidi.cpp
            midi_driver_alsamidi.h
    )
    # [[[end]]]
    target_link_libraries(driver-alsa-midi PUBLIC linuxbsd-headers ${ALSA_LIBRARIES} godot-error)
    target_compile_definitions(driver-alsa-midi PUBLIC ALSAMIDI_ENABLED=1)
endif ()