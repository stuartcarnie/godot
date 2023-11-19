if(${WANT_COREMIDI})
    godot_add_library(driver-coremidi FOLDER drivers STATIC)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(driver-coremidi PRIVATE')
    #   for file in list_files(cog, '.'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(driver-coremidi PRIVATE
            midi_driver_coremidi.cpp
            midi_driver_coremidi.h
    )
    # [[[end]]]

    target_compile_definitions(driver-coremidi PUBLIC COREMIDI_ENABLED=1)
    target_link_libraries(driver-coremidi PUBLIC godot-error)
endif()