if (${WANT_ALSA} AND ${ALSA_FOUND})
    godot_add_library(alsamidi STATIC)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(alsamidi PRIVATE')
    #   for file in list_files(cog, '.'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(alsamidi PRIVATE
            midi_driver_alsamidi.cpp
            midi_driver_alsamidi.h
    )
    # [[[end]]]
    target_link_libraries(alsamidi PUBLIC linuxbsd-headers ${ALSA_LIBRARIES} core)
    target_compile_definitions(alsamidi PUBLIC ALSAMIDI_ENABLED)
endif ()
