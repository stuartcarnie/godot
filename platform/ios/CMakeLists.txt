godot_add_library(ios)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(ios PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(ios PRIVATE
        device_metrics.h
        device_metrics.mm
        display_layer_ios.h
        display_layer_ios.mm
        display_server_ios.h
        display_server_ios.mm
        godot_view_ios.h
        godot_view_ios.mm
        ios.h
        main_ios.mm
        os_ios.h
        os_ios.mm
        platform_config.h
        platform_gl.h
        platform_thread.h
)
# [[[end]]]

set(_apple_embedded_plugins_stub "${CMAKE_BINARY_DIR}/generated/apple_embedded_plugins.gen.cpp")
file(WRITE "${_apple_embedded_plugins_stub}" [=[
// Auto-generated stub for CMake builds.
void godot_apple_embedded_plugins_initialize() {}
void godot_apple_embedded_plugins_deinitialize() {}
]=])

target_sources(ios PRIVATE
        ${_apple_embedded_plugins_stub}
)

if (${WANT_OPENGL})
    target_link_libraries(ios PUBLIC OpenGLES3::ES3)
    target_link_libraries(ios PUBLIC gles3)
    if (WANT_ANGLE)
        target_link_libraries(ios PRIVATE angle)
    endif ()
    target_compile_definitions(ios PRIVATE GLES3_ENABLED)
endif ()

if (${WANT_VULKAN} AND ${VULKAN_FOUND})
    target_link_libraries(ios PUBLIC driver-vulkan vulkan
            $<LINK_LIBRARY:FRAMEWORK,${MOLTENVK_FOUND}>)
    target_compile_definitions(ios PRIVATE VULKAN_ENABLED)
endif ()

target_link_libraries(ios
        PUBLIC
        $<$<BOOL:${WANT_METAL}>:metal>
)
target_compile_definitions(ios
        PUBLIC
        $<$<BOOL:${WANT_METAL}>:METAL_ENABLED>
        $<${WANT_COREAUDIO}:COREAUDIO_ENABLED>
        $<${WANT_COREMIDI}:COREMIDI_ENABLED>
        $<${WANT_SDL}:SDL_ENABLED>
)
target_compile_options(ios PRIVATE
        $<$<COMPILE_LANGUAGE:CXX,OBJCXX>:-fmodules -fcxx-modules>)

#add_custom_target(libgodot
#        COMMAND libtool -static -o libgodot.a $<TARGET_FILE:ios>
#)
#add_dependencies(libgodot ${GODOT_LIBRARIES})

target_link_libraries(ios PUBLIC
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
        $<LINK_LIBRARY:FRAMEWORK,Foundation.framework>
        $<LINK_LIBRARY:FRAMEWORK,CoreGraphics.framework>
        $<LINK_LIBRARY:FRAMEWORK,CoreFoundation.framework>
        $<LINK_LIBRARY:FRAMEWORK,UIKit.framework>
)

# Set the SDKROOT Xcode properties to a Xcode-friendly value (the SDK_NAME, E.g, iphoneos)
# This way, Xcode will automatically switch between the simulator and device SDK when building.
set(CMAKE_XCODE_ATTRIBUTE_SDKROOT "iphoneos")
# Force to not build just one ARCH, but all!
set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "YES")
