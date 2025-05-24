godot_add_library(drivers STATIC)

if (WANT_ALSA)
    find_package(ALSA)
endif ()
if (WANT_PULSEAUDIO)
    find_package(PulseAudio)
endif ()

add_subdirectory(alsa)
add_subdirectory(alsamidi)
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_subdirectory(apple)
endif ()
if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_subdirectory(apple_embedded)
endif ()
add_subdirectory(coreaudio)
add_subdirectory(coremidi)
if (${WANT_OPENGL} AND ${OPENGL_FOUND})
    add_subdirectory(gles3)
    if (WANT_GLAD)
        add_subdirectory(gl_context)
    endif ()
    add_subdirectory(egl)
endif ()
if (ENABLE_METAL)
    add_subdirectory(metal)
endif ()
add_subdirectory(png)
add_subdirectory(pulseaudio)
add_subdirectory(unix)
add_subdirectory(vulkan)
add_subdirectory(wasapi)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_subdirectory(windows)
    add_subdirectory(winmidi)
endif ()


# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(drivers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(drivers PRIVATE
        register_driver_types.cpp
        register_driver_types.h
)
# [[[end]]]
target_link_libraries(drivers PUBLIC core)

group_target_sources(drivers)
