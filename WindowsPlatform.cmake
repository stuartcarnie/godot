#Detect and enable all the possible libraries
#DO NOT MARK THINGS REQUIRED, break within the module that requires it

set(GODOT_ENGINE_WINDOWS_TARGET_WIN_VERSION "0x0601" CACHE STRING "Targeted Windows version, >= 0x0601 (Windows 7)")
set(GODOT_ENGINE_WINDOWS_SUBSYSTEM "gui" CACHE STRING "Set to 'gui' or 'console'")
option(GODOT_ENGINE_WINDOWS_STATIC_CPP "Link MinGW/MSVC C++ runtime libraries statically" On)

add_link_options(/SUBSYSTEM:WINDOWS)

if(GODOT_ENGINE_WINDOWS_SUBSYSTEM STREQUAL "gui")
elseif(GODOT_ENGINE_WINDOWS_SUBSYSTEM STREQUAL "console")
	add_compile_definitions(WINDOWS_SUBSYSTEM_CONSOLE)
else()
	message(FATAL_ERROR "Invalid option for windows subsystem: ${GODOT_ENGINE_WINDOWS_SUBSYSTEM}. Should be 'gui' or 'console'.")
endif()

add_compile_options(/Gd /GR /nologo)
add_compile_options(/utf-8)
add_compile_options(/TP)
add_compile_options(/bigobj)

# TODO: Move WASAPI_ENABLED and WINMIDI_ENABLED to a more logical spot
add_compile_definitions(WINDOWS_ENABLED WASAPI_ENABLED WINMIDI_ENABLED TYPED_METHOD_BIND WIN32 MSVC)
add_compile_definitions(WINVER=${GODOT_ENGINE_WINDOWS_TARGET_WIN_VERSION})
add_compile_definitions(_WIN32_WINNT=${GODOT_ENGINE_WINDOWS_TARGET_WIN_VERSION})
add_compile_definitions(NOMINMAX)
add_compile_definitions(_WIN64)

# Turn off LTO -- this may be helpful later if using clang/gcc
set_property(GLOBAL PROPERTY INTERPROCEDURAL_OPTIMIZATION False)

# TODO: Support ASAN

add_link_options(/STACK:8388608)

set(NEED_WINDOWS_DRIVER 1)
set(WANT_VULKAN 1)
set(WANT_OPENGL 1)
set(WANT_XAUDIO2 1)
set(WANT_WINMIDI 1)
set(WANT_WASAPI 1)
set(WANT_GLAD 1)

