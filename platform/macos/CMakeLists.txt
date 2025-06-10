godot_add_library(macos FOLDER platform)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(macos PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(macos PRIVATE
        crash_handler_macos.h
        crash_handler_macos.mm
        dir_access_macos.h
        dir_access_macos.mm
        display_server_embedded.h
        display_server_embedded.mm
        display_server_macos.h
        display_server_macos.mm
        display_server_macos_base.h
        display_server_macos_base.mm
        embedded_debugger.h
        embedded_debugger.mm
        embedded_gl_manager.h
        embedded_gl_manager.mm
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
        key_mapping_macos.h
        key_mapping_macos.mm
        macos_quartz_core_spi.h
        macos_terminal_logger.h
        macos_terminal_logger.mm
        native_menu_macos.h
        native_menu_macos.mm
        os_macos.h
        os_macos.mm
        platform_config.h
        platform_gl.h
        platform_thread.h
        rendering_context_driver_vulkan_macos.h
        rendering_context_driver_vulkan_macos.mm
        tts_macos.h
        tts_macos.mm
)
# [[[end]]]

if (WANT_EDITOR)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(macos PRIVATE')
    #   for file in list_files(cog, 'editor'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(macos PRIVATE
            editor/embedded_game_view_plugin.h
            editor/embedded_game_view_plugin.mm
            editor/embedded_process_macos.h
            editor/embedded_process_macos.mm
    )
    # [[[end]]]
endif ()

if(${WANT_OPENGL} AND ${OPENGL_FOUND})
    target_link_libraries(macos PUBLIC OpenGL::GL)
    target_link_libraries(macos PUBLIC gles3 glad)
    target_link_libraries(macos PRIVATE angle)
    target_compile_definitions(macos PRIVATE GLES3_ENABLED=1)
endif()

if(${WANT_VULKAN} AND ${VULKAN_FOUND})
    target_link_libraries(macos PUBLIC driver-vulkan vulkan MoltenVK)
    target_compile_definitions(macos PRIVATE VULKAN_ENABLED=1)
endif()

target_link_libraries(macos
        PUBLIC
        $<$<BOOL:${WANT_METAL}>:metal>
)
target_compile_definitions(macos
        PUBLIC
        $<$<BOOL:${WANT_METAL}>:METAL_ENABLED=1 RD_ENABLED=1>
        $<${WANT_COREAUDIO}:COREAUDIO_ENABLED=1>
        $<${WANT_COREMIDI}:COREMIDI_ENABLED=1>
)

target_compile_options(macos PRIVATE -Wunguarded-availability)

target_link_libraries(macos PUBLIC
        $<LINK_LIBRARY:FRAMEWORK,Cocoa.framework>
        $<LINK_LIBRARY:FRAMEWORK,Carbon.framework>
        $<LINK_LIBRARY:FRAMEWORK,AudioUnit.framework>
        $<LINK_LIBRARY:FRAMEWORK,CoreAudio.framework>
        $<LINK_LIBRARY:FRAMEWORK,CoreMIDI.framework>
        $<LINK_LIBRARY:FRAMEWORK,IOKit.framework>
        $<LINK_LIBRARY:FRAMEWORK,GameController.framework>
        $<LINK_LIBRARY:FRAMEWORK,CoreHaptics.framework>
        $<LINK_LIBRARY:FRAMEWORK,CoreVideo.framework>
        $<LINK_LIBRARY:FRAMEWORK,AVFoundation.framework>
        $<LINK_LIBRARY:FRAMEWORK,CoreMedia.framework>
        $<LINK_LIBRARY:FRAMEWORK,QuartzCore.framework>
        $<LINK_LIBRARY:FRAMEWORK,Security.framework>
        $<LINK_LIBRARY:FRAMEWORK,Metal.framework>
        $<LINK_LIBRARY:FRAMEWORK,MetalKit.framework>
        $<LINK_LIBRARY:FRAMEWORK,MetalFX.framework>
        $<LINK_LIBRARY:FRAMEWORK,IOSurface.framework>
        $<LINK_LIBRARY:FRAMEWORK,UniformTypeIdentifiers.framework>
)

group_target_sources(macos)
