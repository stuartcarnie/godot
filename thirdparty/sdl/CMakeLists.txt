godot_add_library(sdl3 FOLDER modules STATIC)

target_sources(sdl3 PRIVATE
        SDL.c
        SDL_assert.c
        SDL_error.c
        SDL_guid.c
        SDL_hashtable.c
        SDL_hints.c
        SDL_list.c
        SDL_log.c
        SDL_properties.c
        SDL_utils.c
        atomic/SDL_atomic.c
        atomic/SDL_spinlock.c
        events/SDL_events.c
        events/SDL_eventwatch.c
        haptic/SDL_haptic.c
        io/SDL_iostream.c
        joystick/SDL_gamepad.c
        joystick/SDL_joystick.c
        joystick/SDL_steam_virtual_gamepad.c
        joystick/controller_type.c
        libm/e_atan2.c
        libm/e_exp.c
        libm/e_fmod.c
        libm/e_log.c
        libm/e_log10.c
        libm/e_pow.c
        libm/e_rem_pio2.c
        libm/e_sqrt.c
        libm/k_cos.c
        libm/k_rem_pio2.c
        libm/k_sin.c
        libm/k_tan.c
        libm/s_atan.c
        libm/s_copysign.c
        libm/s_cos.c
        libm/s_fabs.c
        libm/s_floor.c
        libm/s_isinf.c
        libm/s_isinff.c
        libm/s_isnan.c
        libm/s_isnanf.c
        libm/s_modf.c
        libm/s_scalbn.c
        libm/s_sin.c
        libm/s_tan.c
        sensor/SDL_sensor.c
        sensor/dummy/SDL_dummysensor.c
        stdlib/SDL_crc16.c
        stdlib/SDL_crc32.c
        stdlib/SDL_getenv.c
        stdlib/SDL_iconv.c
        stdlib/SDL_malloc.c
        stdlib/SDL_memcpy.c
        stdlib/SDL_memmove.c
        stdlib/SDL_memset.c
        stdlib/SDL_mslibc.c
        stdlib/SDL_murmur3.c
        stdlib/SDL_qsort.c
        stdlib/SDL_random.c
        stdlib/SDL_stdlib.c
        stdlib/SDL_string.c
        stdlib/SDL_strtokr.c
        thread/SDL_thread.c
        timer/SDL_timer.c

        # HIDAPI
        hidapi/SDL_hidapi.c
        joystick/hidapi/SDL_hidapi_combined.c
        joystick/hidapi/SDL_hidapi_gamecube.c
        joystick/hidapi/SDL_hidapijoystick.c
        joystick/hidapi/SDL_hidapi_luna.c
        joystick/hidapi/SDL_hidapi_ps3.c
        joystick/hidapi/SDL_hidapi_ps4.c
        joystick/hidapi/SDL_hidapi_ps5.c
        joystick/hidapi/SDL_hidapi_rumble.c
        joystick/hidapi/SDL_hidapi_shield.c
        joystick/hidapi/SDL_hidapi_stadia.c
        joystick/hidapi/SDL_hidapi_steam.c
        joystick/hidapi/SDL_hidapi_steamdeck.c
        joystick/hidapi/SDL_hidapi_steam_hori.c
        joystick/hidapi/SDL_hidapi_switch.c
        joystick/hidapi/SDL_hidapi_wii.c
        joystick/hidapi/SDL_hidapi_xbox360.c
        joystick/hidapi/SDL_hidapi_xbox360w.c
        joystick/hidapi/SDL_hidapi_xboxone.c
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_sources(sdl3 PRIVATE
            core/linux/SDL_dbus.c
            core/linux/SDL_evdev.c
            core/linux/SDL_evdev_capabilities.c
            core/linux/SDL_evdev_kbd.c
            core/linux/SDL_fcitx.c
            core/linux/SDL_ibus.c
            core/linux/SDL_ime.c
            core/linux/SDL_system_theme.c
            core/linux/SDL_threadprio.c
            core/linux/SDL_udev.c
            core/unix/SDL_appid.c
            core/unix/SDL_poll.c
            haptic/linux/SDL_syshaptic.c
            joystick/linux/SDL_sysjoystick.c
            loadso/dlopen/SDL_sysloadso.c
            thread/pthread/SDL_syscond.c
            thread/pthread/SDL_sysmutex.c
            thread/pthread/SDL_sysrwlock.c
            thread/pthread/SDL_syssem.c
            thread/pthread/SDL_systhread.c
            thread/pthread/SDL_systls.c
            timer/unix/SDL_systimer.c
    )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_sources(sdl3 PRIVATE
            core/unix/SDL_appid.c
            core/unix/SDL_poll.c
            haptic/darwin/SDL_syshaptic.c
            joystick/darwin/SDL_iokitjoystick.c
            joystick/apple/SDL_mfijoystick.m
            thread/pthread/SDL_syscond.c
            thread/pthread/SDL_sysmutex.c
            thread/pthread/SDL_sysrwlock.c
            thread/pthread/SDL_syssem.c
            thread/pthread/SDL_systhread.c
            thread/pthread/SDL_systls.c
            timer/unix/SDL_systimer.c
    )
#elseif (CMAKE_SYSTEM_NAME STREQUAL "iOS")
#    target_link_libraries(godot-module-ktx PRIVATE ios)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_sources(sdl3 PRIVATE
            core/windows/SDL_gameinput.c
            core/windows/SDL_hid.c
            core/windows/SDL_immdevice.c
            core/windows/SDL_windows.c
            core/windows/SDL_xinput.c
            core/windows/pch.c
            haptic/windows/SDL_dinputhaptic.c
            haptic/windows/SDL_windowshaptic.c
            joystick/windows/SDL_dinputjoystick.c
            joystick/windows/SDL_rawinputjoystick.c
            joystick/windows/SDL_windows_gaming_input.c
            joystick/windows/SDL_windowsjoystick.c
            joystick/windows/SDL_xinputjoystick.c
            thread/generic/SDL_syscond.c
            thread/generic/SDL_sysrwlock.c
            thread/windows/SDL_syscond_cv.c
            thread/windows/SDL_sysmutex.c
            thread/windows/SDL_sysrwlock_srw.c
            thread/windows/SDL_syssem.c
            thread/windows/SDL_systhread.c
            thread/windows/SDL_systls.c
            timer/windows/SDL_systimer.c
    )
else ()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif ()

target_compile_definitions(sdl3
        PRIVATE
        SDL_PLATFORM_PRIVATE
        $<$<PLATFORM_ID:Linux>:SDL_PLATFORM_LINUX>
        $<$<PLATFORM_ID:Darwin>:SDL_PLATFORM_MACOS>
        $<$<PLATFORM_ID:iOS>:SDL_PLATFORM_IOS>
        $<$<PLATFORM_ID:Windows>:SDL_PLATFORM_WINDOWS>
)
target_include_directories(sdl3
        PUBLIC
        .
        include
        include/build_config
        ${CMAKE_SOURCE_DIR}/drivers/sdl # SDL_build_config_private.h
)
