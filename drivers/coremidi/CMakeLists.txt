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
            midi_driver_coremidi.cpp
            midi_driver_coremidi.h
    )
    # [[[end]]]

    target_compile_definitions(coremidi PUBLIC COREMIDI_ENABLED=1)
    target_link_libraries(coremidi PUBLIC godot-error)

    group_target_sources(coremidi)
endif()
