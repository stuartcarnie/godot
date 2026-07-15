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

set(_apple_embedded_plugins_stub "${CMAKE_BINARY_DIR}/generated/apple_embedded_plugins.gen.cpp")
file(WRITE "${_apple_embedded_plugins_stub}" [=[
// Auto-generated stub for CMake builds.
void godot_apple_embedded_plugins_initialize() {}
void godot_apple_embedded_plugins_deinitialize() {}
]=])

target_sources(visionos PRIVATE
        ${_apple_embedded_plugins_stub}
)

target_link_libraries(visionos
        PUBLIC
        $<$<BOOL:${WANT_METAL}>:metal>
)
target_compile_definitions(visionos
        PUBLIC
        $<$<BOOL:${WANT_METAL}>:METAL_ENABLED>
        $<${WANT_COREAUDIO}:COREAUDIO_ENABLED>
        $<${WANT_COREMIDI}:COREMIDI_ENABLED>
        $<${WANT_SDL}:SDL_ENABLED>
)
target_compile_options(visionos
        PRIVATE
        $<$<COMPILE_LANGUAGE:CXX,OBJCXX>:-fmodules -fcxx-modules>
        $<$<COMPILE_LANGUAGE:Swift>:-import-objc-header ${CMAKE_CURRENT_SOURCE_DIR}/bridging_header_visionos.h>
        $<$<COMPILE_LANGUAGE:Swift>:-cxx-interoperability-mode=default>
)


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
