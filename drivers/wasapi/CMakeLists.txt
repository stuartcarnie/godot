if (${WANT_WASAPI})
    godot_add_library(driver-wasapi FOLDER drivers STATIC)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(driver-wasapi PRIVATE')
    #   for file in list_files(cog, '.'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(driver-wasapi PRIVATE
            audio_driver_wasapi.cpp
            audio_driver_wasapi.h
    )
    # [[[end]]]

    target_link_libraries(driver-wasapi PUBLIC godot-core)
    target_include_directories(driver-wasapi PRIVATE ${GODOT_ENGINE_ROOT_DIRECTORY})
endif ()