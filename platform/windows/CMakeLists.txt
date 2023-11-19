set(PLATFORM_WINDOWS_SOURCE_DIR ${GODOT_SOURCE_ROOT_PLATFORM_DIR}/windows)
set(PLATFORM_WINDOWS_SOURCE
	${PLATFORM_WINDOWS_SOURCE_DIR}/export/export.cpp
	${PLATFORM_WINDOWS_SOURCE_DIR}/export/export.h
	${PLATFORM_WINDOWS_SOURCE_DIR}/export/export_plugin.cpp
	${PLATFORM_WINDOWS_SOURCE_DIR}/export/export_plugin.h
	${PLATFORM_WINDOWS_SOURCE_DIR}/crash_handler_windows.cpp
	${PLATFORM_WINDOWS_SOURCE_DIR}/crash_handler_windows.h
	${PLATFORM_WINDOWS_SOURCE_DIR}/display_server_windows.cpp
	${PLATFORM_WINDOWS_SOURCE_DIR}/display_server_windows.h
	${PLATFORM_WINDOWS_SOURCE_DIR}/gl_manager_windows.cpp
	${PLATFORM_WINDOWS_SOURCE_DIR}/gl_manager_windows.h
	${PLATFORM_WINDOWS_SOURCE_DIR}/godot.ico
	${PLATFORM_WINDOWS_SOURCE_DIR}/godot.natvis
	${PLATFORM_WINDOWS_SOURCE_DIR}/godot_console.ico
	${PLATFORM_WINDOWS_SOURCE_DIR}/godot_res.rc
	${PLATFORM_WINDOWS_SOURCE_DIR}/godot_res_wrap.rc
	${PLATFORM_WINDOWS_SOURCE_DIR}/godot_windows.cpp
	${PLATFORM_WINDOWS_SOURCE_DIR}/joypad_windows.cpp
	${PLATFORM_WINDOWS_SOURCE_DIR}/joypad_windows.h
	${PLATFORM_WINDOWS_SOURCE_DIR}/key_mapping_windows.cpp
	${PLATFORM_WINDOWS_SOURCE_DIR}/key_mapping_windows.h
	${PLATFORM_WINDOWS_SOURCE_DIR}/lang_table.h
	${PLATFORM_WINDOWS_SOURCE_DIR}/os_windows.cpp
	${PLATFORM_WINDOWS_SOURCE_DIR}/os_windows.h
	${PLATFORM_WINDOWS_SOURCE_DIR}/platform_config.h
	${PLATFORM_WINDOWS_SOURCE_DIR}/platform_windows_builders.py
	${PLATFORM_WINDOWS_SOURCE_DIR}/README.md
	${PLATFORM_WINDOWS_SOURCE_DIR}/SCsub
	${PLATFORM_WINDOWS_SOURCE_DIR}/tts_windows.cpp
	${PLATFORM_WINDOWS_SOURCE_DIR}/tts_windows.h
	${PLATFORM_WINDOWS_SOURCE_DIR}/vulkan_context_win.cpp
	${PLATFORM_WINDOWS_SOURCE_DIR}/vulkan_context_win.h
	${PLATFORM_WINDOWS_SOURCE_DIR}/windows_terminal_logger.cpp
	${PLATFORM_WINDOWS_SOURCE_DIR}/windows_terminal_logger.h
	)

#Generate the Logo and Run icon
set(PLATFORM_WINDOWS_LOGO_SVG_SOURCE ${PLATFORM_WINDOWS_SOURCE_DIR}/logo.svg)
set(PLATFORM_WINDOWS_LOGO_SVG_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/platform/windows/logo_svg.gen.h)
generate_platform_logos_and_run_icon(windows logo ${PLATFORM_WINDOWS_LOGO_SVG_SOURCE} ${PLATFORM_WINDOWS_LOGO_SVG_OUTPUT})
set(PLATFORM_WINDOWS_RUN_SVG_SOURCE ${PLATFORM_WINDOWS_SOURCE_DIR}/run_icon.svg)
set(PLATFORM_WINDOWS_RUN_SVG_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/platform/windows/run_icon_svg.gen.h)
generate_platform_logos_and_run_icon(windows run_icon ${PLATFORM_WINDOWS_RUN_SVG_SOURCE} ${PLATFORM_WINDOWS_RUN_SVG_OUTPUT})

#Find the package to link and enable/disable
godot_add_library(platform-windows FOLDER platform STATIC ${PLATFORM_WINDOWS_SOURCE} ${PLATFORM_WINDOWS_LOGO_SVG_OUTPUT} ${PLATFORM_WINDOWS_RUN_SVG_OUTPUT})
target_include_directories(platform-windows PRIVATE
                                             ${GODOT_ENGINE_ROOT_DIRECTORY}
                                             ${DBUS_INCLUDE_DIRS})
target_include_directories(platform-windows PUBLIC ${PLATFORM_WINDOWS_SOURCE_DIR})
set_property(TARGET platform-windows PROPERTY CXX_STANDARD 17)
target_link_libraries(platform-windows PUBLIC driver-windows driver-wasapi zlib)
# Libraries provided by Windows SDK and DirectX
target_link_libraries(platform-windows PUBLIC dinput8 dxguid dwrite avrt dwmapi imm32 wbemuuid)

if(${WANT_OPENGL} AND ${OPENGL_FOUND})
    target_link_libraries(platform-windows PUBLIC OpenGL::GL)
	target_link_libraries(platform-windows PUBLIC driver-gles3)
endif()

if(${WANT_VULKAN} AND ${VULKAN_FOUND})
    target_link_libraries(platform-windows PUBLIC vulkan glad volk)
	target_link_libraries(platform-windows PUBLIC driver-vulkan)

	# We rely on Volk alone.
    #target_link_libraries (platform-windows PUBLIC ${Vulkan_LIBRARY})
endif()

