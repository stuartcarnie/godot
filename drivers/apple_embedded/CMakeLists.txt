godot_add_library(apple_embedded FOLDER drivers STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(apple_embedded PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(apple_embedded PRIVATE
        app_delegate_service.h
        app_delegate_service.mm
        apple_embedded.h
        apple_embedded.mm
        display_layer_apple_embedded.h
        display_server_apple_embedded.h
        display_server_apple_embedded.mm
        godot_app_delegate.h
        godot_app_delegate.mm
        godot_view_apple_embedded.h
        godot_view_apple_embedded.mm
        godot_view_renderer.h
        godot_view_renderer.mm
        key_mapping_apple_embedded.h
        key_mapping_apple_embedded.mm
        keyboard_input_view.h
        keyboard_input_view.mm
        main_utilities.h
        main_utilities.mm
        os_apple_embedded.h
        os_apple_embedded.mm
        platform_config.h
        rendering_context_driver_vulkan_apple_embedded.h
        rendering_context_driver_vulkan_apple_embedded.mm
        terminal_logger_apple_embedded.h
        terminal_logger_apple_embedded.mm
        tts_apple_embedded.h
        tts_apple_embedded.mm
        view_controller.h
        view_controller.mm
)
# [[[end]]]

target_compile_definitions(apple_embedded PUBLIC COREAUDIO_ENABLED=1)
target_compile_features(apple_embedded PRIVATE cxx_std_20)
set_target_properties(apple_embedded PROPERTIES XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD c++20)
target_link_libraries(apple_embedded PRIVATE core)

group_target_sources(apple_embedded)
