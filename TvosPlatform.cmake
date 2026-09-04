set(NEED_UNIX_DRIVER 1)
option(ENABLE_METAL "Enable the Metal renderer" ON)
option(ENABLE_COREAUDIO "Enable the CoreAudio driver" ON)
set(GODOT_PLATFORM "tvos")
add_compile_definitions(TVOS_ENABLED APPLE_EMBEDDED_ENABLED)

add_compile_definitions(UNIX_ENABLED)
