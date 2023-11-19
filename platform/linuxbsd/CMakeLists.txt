cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

set(PLATFORM_LINUXBSD_SOURCE_DIR ${GODOT_SOURCE_ROOT_PLATFORM_DIR}/linuxbsd)
set(PLATFORM_LINUXBSD_SOURCE
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/crash_handler_linuxbsd.cpp
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/crash_handler_linuxbsd.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/dbus-so_wrap.c
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/dbus-so_wrap.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/detect.py
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/export/export_plugin.cpp
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/export/export.cpp
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/export/export.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/export/export_plugin.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/fontconfig-so_wrap.c
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/fontconfig-so_wrap.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/freedesktop_portal_desktop.cpp
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/freedesktop_portal_desktop.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/freedesktop_screensaver.cpp
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/freedesktop_screensaver.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/godot_linuxbsd.cpp
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/joypad_linux.cpp
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/joypad_linux.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/libudev-so_wrap.c
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/libudev-so_wrap.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/os_linuxbsd.cpp
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/os_linuxbsd.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/pck_embed.ld
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/pck_embed.legacy.ld
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/platform_config.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/platform_linuxbsd_builders.py
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/README.md
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/SCsub
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/speechd-so_wrap.c
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/speechd-so_wrap.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/tts_linux.cpp
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/tts_linux.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/key_mapping_x11.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/dynwrappers/xrandr-so_wrap.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/dynwrappers/xlib-so_wrap.c
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/dynwrappers/xrandr-so_wrap.c
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/dynwrappers/xrender-so_wrap.c
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/dynwrappers/xext-so_wrap.c
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/dynwrappers/xrender-so_wrap.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/dynwrappers/xinput2-so_wrap.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/dynwrappers/xcursor-so_wrap.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/dynwrappers/xext-so_wrap.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/dynwrappers/xinput2-so_wrap.c
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/dynwrappers/xlib-so_wrap.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/dynwrappers/xinerama-so_wrap.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/dynwrappers/xcursor-so_wrap.c
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/dynwrappers/xinerama-so_wrap.c
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/key_mapping_x11.cpp
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/gl_manager_x11.cpp
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/display_server_x11.cpp
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/gl_manager_x11.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/display_server_x11.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/detect_prime_x11.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/vulkan_context_x11.cpp
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/SCsub
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/detect_prime_x11.cpp
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/x11/vulkan_context_x11.h
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/xkbcommon-so_wrap.c
    ${PLATFORM_LINUXBSD_SOURCE_DIR}/xkbcommon-so_wrap.h
    )

#Generate the Logo and Run icon
set(PLATFORM_LINUXBSD_LOGO_SVG_SOURCE ${PLATFORM_LINUXBSD_SOURCE_DIR}/logo.svg)
set(PLATFORM_LINUXBSD_LOGO_SVG_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/platform/linuxbsd/logo_svg.gen.h)
generate_platform_logos_and_run_icon(linuxbsd logo ${PLATFORM_LINUXBSD_LOGO_SVG_SOURCE} ${PLATFORM_LINUXBSD_LOGO_SVG_OUTPUT})
set(PLATFORM_LINUXBSD_RUN_SVG_SOURCE ${PLATFORM_LINUXBSD_SOURCE_DIR}/run_icon.svg)
set(PLATFORM_LINUXBSD_RUN_SVG_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/platform/linuxbsd/run_icon_svg.gen.h)
generate_platform_logos_and_run_icon(linuxbsd run_icon ${PLATFORM_LINUXBSD_RUN_SVG_SOURCE} ${PLATFORM_LINUXBSD_RUN_SVG_OUTPUT})


#Find the package to link and enable/disable
godot_add_library(platform-linuxbsd FOLDER platform STATIC ${PLATFORM_LINUXBSD_SOURCE} ${PLATFORM_LINUXBSD_LOGO_SVG_OUTPUT} ${PLATFORM_LINUXBSD_RUN_SVG_OUTPUT})
target_include_directories(platform-linuxbsd PRIVATE
                                             ${GODOT_ENGINE_ROOT_DIRECTORY}
                                             ${DBUS_INCLUDE_DIRS}
                                             ${GODOT_FAKEGEN_ROOT_DIR}
                                             ${CMAKE_BINARY_INCLUDE_DIR}/platform/linuxbsd)
target_include_directories(platform-linuxbsd PUBLIC ${PLATFORM_LINUXBSD_SOURCE_DIR})

#Depends on a single generated shader from godot-server-renderer
target_link_libraries(platform-linuxbsd PUBLIC ${DBUS_LIBRARIES} driver-unix godot-server-renderer)

#Required to always be enabled on linux 
target_compile_definitions(platform-linuxbsd PUBLIC JOYDEV_ENABLED=1 ALSAMIDI_ENABLED=1)

if(${WANT_ALSA} AND ${ALSA_FOUND})
    target_compile_definitions(platform-linuxbsd PUBLIC ALSA_ENABLED=1)
    target_link_libraries(platform-linuxbsd PUBLIC driver-alsa)
    target_link_libraries(platform-linuxbsd PUBLIC driver-alsa-midi)
endif()

if(${WANT_PULSEAUDIO} AND ${PULSEAUDIO_FOUND})
    target_compile_definitions(platform-linuxbsd PUBLIC PULSEAUDIO_ENABLED=1)
    target_link_libraries(platform-linuxbsd PUBLIC driver-pulseaudio)
endif()


if(${WANT_X11} AND ${X11_FOUND})
    target_compile_definitions(platform-linuxbsd PUBLIC X11_ENABLED=1)
endif()

if(${FOUND_MNTENT})
    target_compile_definitions(platform-linuxbsd PUBLIC HAVE_MNTENT=1)
endif()

if(${WANT_FONTCONFIG} AND ${Fontconfig_FOUND})
    target_compile_definitions(platform-linuxbsd PUBLIC FONTCONFIG_ENABLED=1)
endif()

if(${WANT_VULKAN} AND ${Vulkan_FOUND})
    target_compile_definitions(platform-linuxbsd PUBLIC VULKAN_ENABLED=1 USE_VOLK=1)
    target_link_libraries(platform-linuxbsd PUBLIC vulkan glad volk)
    target_link_libraries (platform-linuxbsd PUBLIC ${Vulkan_LIBRARY})
    target_link_libraries(platform-linuxbsd PUBLIC driver-vulkan)
endif()

if(${WANT_OPENGL} AND ${OPENGL_FOUND})
    target_compile_definitions(platform-linuxbsd PUBLIC GLES3_ENABLED=1)
    target_link_libraries(platform-linuxbsd PUBLIC OpenGL::GL)
    target_link_libraries(platform-linuxbsd PUBLIC driver-gles3)
endif()

if(${WANT_X11} AND ${X11_FOUND})
    target_link_libraries(platform-linuxbsd PUBLIC ${X11_LIBRARIES})
endif()

if(${WANT_DBUS} AND ${DBUS_FOUND})
    target_compile_definitions(platform-linuxbsd PUBLIC DBUS_ENABLED=1)
endif()
