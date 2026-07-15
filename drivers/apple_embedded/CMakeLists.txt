godot_add_library(apple_embedded FOLDER drivers STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(apple_embedded PRIVATE')
#   for file in list_files(cog, '.', exts=['c', 'cpp', 'h', 'mm', 'swift']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(apple_embedded PRIVATE
        app.swift
        app_delegate_service.h
        app_delegate_service.mm
        apple_embedded.h
        apple_embedded.mm
        bridging_header_apple_embedded.h
        display_layer_apple_embedded.h
        display_server_apple_embedded.h
        display_server_apple_embedded.mm
        godot_app_delegate.h
        godot_app_delegate.mm
        godot_keyboard_input_view.h
        godot_keyboard_input_view.mm
        godot_view_apple_embedded.h
        godot_view_apple_embedded.mm
        godot_view_controller.h
        godot_view_controller.mm
        godot_view_renderer.h
        godot_view_renderer.mm
        key_mapping_apple_embedded.h
        key_mapping_apple_embedded.mm
        main_utilities.h
        main_utilities.mm
        os_apple_embedded.h
        os_apple_embedded.mm
        platform_config.h
        rendering_context_driver_vulkan_apple_embedded.h
        rendering_context_driver_vulkan_apple_embedded.mm
        tts_apple_embedded.h
        tts_apple_embedded.mm
)
# [[[end]]]

target_compile_definitions(apple_embedded
        PUBLIC
        COREAUDIO_ENABLED
        $<${WANT_SDL}:SDL_ENABLED>
)
target_compile_features(apple_embedded PRIVATE cxx_std_20)
set_target_properties(apple_embedded PROPERTIES
        XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD c++20
        XCODE_ATTRIBUTE_SWIFT_OBJC_BRIDGING_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/bridging_header_apple_embedded.h"
        XCODE_ATTRIBUTE_SWIFT_OBJC_INTEROP_MODE "objcxx"
)
target_link_libraries(apple_embedded PRIVATE core)
target_compile_options(apple_embedded
        PRIVATE
        $<$<COMPILE_LANGUAGE:CXX,OBJCXX>:-fmodules -fcxx-modules>
        $<$<COMPILE_LANGUAGE:Swift>:-import-objc-header ${CMAKE_CURRENT_SOURCE_DIR}/bridging_header_apple_embedded.h>
        $<$<COMPILE_LANGUAGE:Swift>:-cxx-interoperability-mode=default>
)

group_target_sources(apple_embedded)
