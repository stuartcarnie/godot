if(${WANT_COREAUDIO})
    godot_add_library(driver-coreaudio FOLDER drivers STATIC)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(driver-coreaudio PRIVATE')
    #   for file in list_files(cog, '.'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(driver-coreaudio PRIVATE
            audio_driver_coreaudio.cpp
            audio_driver_coreaudio.h
    )
    # [[[end]]]

    target_compile_definitions(driver-coreaudio PUBLIC COREAUDIO_ENABLED=1)
    target_link_libraries(driver-coreaudio PUBLIC godot-error)
endif()