godot_add_library(platform-ios)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(platform-ios PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(platform-ios PRIVATE
        app_delegate.h
        app_delegate.mm
        device_metrics.h
        device_metrics.m
        display_layer.h
        display_layer.mm
        display_server_ios.h
        display_server_ios.mm
        godot_app_delegate.h
        godot_app_delegate.m
        godot_ios.mm
        godot_view.h
        godot_view.mm
        godot_view_renderer.h
        godot_view_renderer.mm
        ios.h
        ios.mm
        ios_terminal_logger.h
        ios_terminal_logger.mm
        joypad_ios.h
        joypad_ios.mm
        key_mapping_ios.h
        key_mapping_ios.mm
        keyboard_input_view.h
        keyboard_input_view.mm
        main.m
        os_ios.h
        os_ios.mm
        platform_config.h
        platform_gl.h
        rendering_context_driver_vulkan_ios.h
        rendering_context_driver_vulkan_ios.mm
        tts_ios.h
        tts_ios.mm
        view_controller.h
        view_controller.mm
)
# [[[end]]]

if(${WANT_OPENGL})
    target_link_libraries(platform-ios PUBLIC OpenGLES3::ES3)
    target_link_libraries(platform-ios PUBLIC driver-gles3)
    target_link_libraries(platform-ios PRIVATE angle)
    target_compile_definitions(platform-ios PRIVATE GLES3_ENABLED=1)
endif()

if(${WANT_VULKAN} AND ${VULKAN_FOUND})
    target_link_libraries(platform-ios PUBLIC driver-vulkan vulkan "-framework ${MOLTENVK_FOUND}")
    target_compile_definitions(platform-ios PRIVATE VULKAN_ENABLED=1)
endif()

target_link_libraries(platform-ios
        PUBLIC
        $<$<BOOL:${ENABLE_METAL}>:metal>
)
target_compile_definitions(platform-ios
        PUBLIC
        $<$<BOOL:${ENABLE_METAL}>:METAL_ENABLED=1>
        $<${WANT_COREAUDIO}:COREAUDIO_ENABLED=1>
        $<${WANT_COREMIDI}:COREMIDI_ENABLED=1>
)
target_compile_options(platform-ios PRIVATE -fmodules)

#add_custom_target(libgodot
#        COMMAND libtool -static -o libgodot.a $<TARGET_FILE:platform-ios>
#)
#add_dependencies(libgodot ${GODOT_LIBRARIES})

target_link_libraries(platform-ios PUBLIC
        $<LINK_LIBRARY:FRAMEWORK,CoreHaptics.framework>
        $<LINK_LIBRARY:FRAMEWORK,GameController.framework>
        $<LINK_LIBRARY:FRAMEWORK,AudioToolbox.framework>
        $<LINK_LIBRARY:FRAMEWORK,CoreAudio.framework>
        $<LINK_LIBRARY:FRAMEWORK,CoreVideo.framework>
        $<LINK_LIBRARY:FRAMEWORK,CoreMotion.framework>
        $<LINK_LIBRARY:FRAMEWORK,AVFoundation.framework>
        $<LINK_LIBRARY:FRAMEWORK,CoreMedia.framework>
        $<LINK_LIBRARY:FRAMEWORK,QuartzCore.framework>
        $<LINK_LIBRARY:FRAMEWORK,Security.framework>
        $<LINK_LIBRARY:FRAMEWORK,Metal.framework>
        $<LINK_LIBRARY:FRAMEWORK,MetalKit.framework>
        $<LINK_LIBRARY:FRAMEWORK,IOSurface.framework>
)

# Set the SDKROOT Xcode properties to a Xcode-friendly value (the SDK_NAME, E.g, iphoneos)
# This way, Xcode will automatically switch between the simulator and device SDK when building.
set(CMAKE_XCODE_ATTRIBUTE_SDKROOT "iphoneos")
# Force to not build just one ARCH, but all!
set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "YES")
