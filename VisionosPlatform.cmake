set(NEED_UNIX_DRIVER 1)
set(WANT_VOLK 0)
set(WANT_VULKAN 0)
set(WANT_METAL 1)
set(WANT_OPENGL 0)
set(WANT_GLAD 1)
set(WANT_FONTCONFIG 0)
set(WANT_COREAUDIO 1)
set(WANT_COREMIDI 0)
set(WANT_GLAD 0)
set(WANT_OPENXR 0)
set(GODOT_PLATFORM "visionos")
add_compile_definitions(VISIONOS_ENABLED APPLE_EMBEDDED_ENABLED)

option(ENABLE_METAL "Enable the Metal renderer" ON)

set(VISIONOS_APP_ROLE "window" CACHE STRING "Application scene role")
set_property(CACHE VISIONOS_APP_ROLE PROPERTY STRINGS window immersive)

set(VISIONOS_IMMERSION_STYLE "mixed" CACHE STRING "Immersion style for immersive apps")
set_property(CACHE VISIONOS_IMMERSION_STYLE PROPERTY STRINGS full mixed)

add_compile_definitions(UNIX_ENABLED)
