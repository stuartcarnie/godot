if(${WANT_COREMIDI})
    godot_add_library(coremidi FOLDER drivers STATIC)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(coremidi PRIVATE')
    #   for file in list_files(cog, '.'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(coremidi PRIVATE
            midi_driver_coremidi.h
            midi_driver_coremidi.mm
    )
    # [[[end]]]

    target_compile_definitions(coremidi PUBLIC COREMIDI_ENABLED)
    target_link_libraries(coremidi PUBLIC core)

    group_target_sources(coremidi)
endif()
