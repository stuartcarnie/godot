add_subdirectory(api)
godot_add_library(visionos)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(visionos PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(visionos PRIVATE
        display_layer_visionos.h
        display_layer_visionos.mm
        display_server_visionos.h
        display_server_visionos.mm
        godot_view_visionos.h
        godot_view_visionos.mm
        main_visionos.mm
        os_visionos.h
        os_visionos.mm
        platform_config.h
        platform_thread.h
        visionos.h
)
# [[[end]]]

target_link_libraries(visionos
        PUBLIC
        $<$<BOOL:${WANT_METAL}>:metal>
)
target_compile_definitions(visionos
        PUBLIC
        $<$<BOOL:${WANT_METAL}>:METAL_ENABLED=1>
        $<${WANT_COREAUDIO}:COREAUDIO_ENABLED=1>
        $<${WANT_COREMIDI}:COREMIDI_ENABLED=1>
)
target_compile_options(visionos PRIVATE -fmodules)

#add_custom_target(libgodot
#        COMMAND libtool -static -o libgodot.a $<TARGET_FILE:visionos>
#)
#add_dependencies(libgodot ${GODOT_LIBRARIES})

target_link_libraries(visionos PUBLIC
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
        $<LINK_LIBRARY:FRAMEWORK,MetalFX.framework>
        $<LINK_LIBRARY:FRAMEWORK,IOSurface.framework>
)

# Set the SDKROOT Xcode properties to a Xcode-friendly value (the SDK_NAME, E.g, iphoneos)
# This way, Xcode will automatically switch between the simulator and device SDK when building.
set(CMAKE_XCODE_ATTRIBUTE_SDKROOT "xros")
# Force to not build just one ARCH, but all!
set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "YES")
