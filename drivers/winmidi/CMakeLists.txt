if (${WANT_WINMIDI})
    godot_add_library(driver-winmidi FOLDER drivers STATIC)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(driver-winmidi PRIVATE')
    #   for file in list_files(cog, '.'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(driver-winmidi PRIVATE
            midi_driver_winmidi.cpp
            midi_driver_winmidi.h
    )
    # [[[end]]]

    target_link_libraries(driver-winmidi PUBLIC godot-core)
    target_include_directories(driver-winmidi PRIVATE ${GODOT_ENGINE_ROOT_DIRECTORY})
endif ()