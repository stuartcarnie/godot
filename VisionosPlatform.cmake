set(NEED_UNIX_DRIVER 1)
option(ENABLE_METAL "Enable the Metal renderer" ON)
option(ENABLE_COREAUDIO "Enable the CoreAudio driver" ON)
set(GODOT_PLATFORM "visionos")
add_compile_definitions(VISIONOS_ENABLED APPLE_EMBEDDED_ENABLED)

set(VISIONOS_APP_ROLE "window" CACHE STRING "Application scene role")
set_property(CACHE VISIONOS_APP_ROLE PROPERTY STRINGS window immersive)

set(VISIONOS_IMMERSION_STYLE "mixed" CACHE STRING "Immersion style for immersive apps")
set_property(CACHE VISIONOS_IMMERSION_STYLE PROPERTY STRINGS full mixed)

add_compile_definitions(UNIX_ENABLED)
