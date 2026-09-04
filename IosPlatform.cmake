set(NEED_UNIX_DRIVER 1)
option(ENABLE_METAL "Enable the Metal renderer" ON)
option(ENABLE_VULKAN "Enable the Vulkan renderer" ON)
option(ENABLE_OPENGL "Enable the OpenGL renderer" ON)
option(ENABLE_COREAUDIO "Enable the CoreAudio driver" ON)
set(GODOT_PLATFORM "ios")
add_compile_definitions(IOS_ENABLED APPLE_EMBEDDED_ENABLED)

add_compile_definitions(UNIX_ENABLED)
