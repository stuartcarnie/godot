add_subdirectory(export)

godot_add_library(platform-macos)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(platform-macos PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(platform-macos PRIVATE
        crash_handler_macos.h
        crash_handler_macos.mm
        dir_access_macos.h
        dir_access_macos.mm
        display_server_macos.h
        display_server_macos.mm
        gl_manager_macos_angle.h
        gl_manager_macos_angle.mm
        gl_manager_macos_legacy.h
        gl_manager_macos_legacy.mm
        godot_application.h
        godot_application.mm
        godot_application_delegate.h
        godot_application_delegate.mm
        godot_button_view.h
        godot_button_view.mm
        godot_content_view.h
        godot_content_view.mm
        godot_main_macos.mm
        godot_menu_delegate.h
        godot_menu_delegate.mm
        godot_menu_item.h
        godot_menu_item.mm
        godot_open_save_delegate.h
        godot_open_save_delegate.mm
        godot_status_item.h
        godot_status_item.mm
        godot_window.h
        godot_window.mm
        godot_window_delegate.h
        godot_window_delegate.mm
        joypad_macos.cpp
        joypad_macos.h
        key_mapping_macos.h
        key_mapping_macos.mm
        macos_terminal_logger.h
        macos_terminal_logger.mm
        os_macos.h
        os_macos.mm
        platform_config.h
        platform_gl.h
        rendering_context_driver_vulkan_macos.h
        rendering_context_driver_vulkan_macos.mm
        tts_macos.h
        tts_macos.mm
)
# [[[end]]]

if(${WANT_OPENGL} AND ${OPENGL_FOUND})
    target_link_libraries(platform-macos PUBLIC OpenGL::GL)
    target_link_libraries(platform-macos PUBLIC driver-gles3 glad)
    target_link_libraries(platform-macos PRIVATE angle)
    target_compile_definitions(platform-macos PRIVATE GLES3_ENABLED=1)
endif()

if(${WANT_VULKAN} AND ${VULKAN_FOUND})
    target_link_libraries(platform-macos PUBLIC driver-vulkan vulkan MoltenVK)
    target_compile_definitions(platform-macos PRIVATE VULKAN_ENABLED=1)
endif()

target_link_libraries(platform-macos
        PUBLIC
        $<$<BOOL:${ENABLE_METAL}>:driver-metal>
)
target_compile_definitions(platform-macos
        PUBLIC
        $<$<BOOL:${ENABLE_METAL}>:METAL_ENABLED=1>
        $<${WANT_COREAUDIO}:COREAUDIO_ENABLED=1>
        $<${WANT_COREMIDI}:COREMIDI_ENABLED=1>
)

target_link_libraries(platform-macos PUBLIC
        $<LINK_LIBRARY:FRAMEWORK,Cocoa.framework>
        $<LINK_LIBRARY:FRAMEWORK,Carbon.framework>
        $<LINK_LIBRARY:FRAMEWORK,AudioUnit.framework>
        $<LINK_LIBRARY:FRAMEWORK,CoreAudio.framework>
        $<LINK_LIBRARY:FRAMEWORK,CoreMIDI.framework>
        $<LINK_LIBRARY:FRAMEWORK,IOKit.framework>
        $<LINK_LIBRARY:FRAMEWORK,ForceFeedback.framework>
        $<LINK_LIBRARY:FRAMEWORK,CoreVideo.framework>
        $<LINK_LIBRARY:FRAMEWORK,AVFoundation.framework>
        $<LINK_LIBRARY:FRAMEWORK,CoreMedia.framework>
        $<LINK_LIBRARY:FRAMEWORK,QuartzCore.framework>
        $<LINK_LIBRARY:FRAMEWORK,Security.framework>
        $<LINK_LIBRARY:FRAMEWORK,Metal.framework>
        $<LINK_LIBRARY:FRAMEWORK,MetalKit.framework>
        $<LINK_LIBRARY:FRAMEWORK,IOSurface.framework>
)