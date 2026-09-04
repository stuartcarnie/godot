#Detect and enable all the possible libraries
#DO NOT MARK THINGS REQUIRED, break within the module that requires it

####################################
#mntent
####################################
CHECK_INCLUDE_FILE("mntent.h" FOUND_MNTENT)

set(NEED_UNIX_DRIVER 1)
option(ENABLE_VULKAN "Enable the Vulkan renderer" ON)
option(ENABLE_OPENGL "Enable the OpenGL renderer" ON)
option(ENABLE_ALSA "Enable the ALSA audio driver" ON)
option(ENABLE_GLAD "Enable GLAD OpenGL loader" ON)
option(ENABLE_DBUS "Enable D-Bus support" ON)
option(ENABLE_PULSEAUDIO "Enable the PulseAudio driver" ON)
option(ENABLE_X11 "Enable X11 display server" ON)
option(ENABLE_FONTCONFIG "Enable Fontconfig support" ON)
option(ENABLE_OPENXR "Enable OpenXR" ON)

add_compile_definitions(UNIX_ENABLED)
