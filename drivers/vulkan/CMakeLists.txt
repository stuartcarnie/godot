if (${WANT_VULKAN} AND ${VULKAN_FOUND})
    godot_add_library(driver-vulkan FOLDER drivers STATIC)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(driver-vulkan PRIVATE')
    #   for file in list_files(cog, '.'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(driver-vulkan PRIVATE
            rendering_context_driver_vulkan.cpp
            rendering_context_driver_vulkan.h
            rendering_device_driver_vulkan.cpp
            rendering_device_driver_vulkan.h
            vulkan_hooks.cpp
            vulkan_hooks.h
    )
    # [[[end]]]
    target_link_libraries(driver-vulkan PUBLIC godot-error godot-os vulkan)
    target_compile_definitions(driver-vulkan PUBLIC VULKAN_ENABLED=1 RD_ENABLED=1)
endif ()