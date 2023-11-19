#Detect and enable all the possible libraries
#DO NOT MARK THINGS REQUIRED, break within the module that requires it

####################################
#mntent
####################################
CHECK_INCLUDE_FILE("mntent.h" FOUND_MNTENT)

set(NEED_UNIX_DRIVER 1)
set(WANT_VULKAN 1)
set(WANT_OPENGL 1)
set(WANT_ALSA 1)
set(WANT_VULKAN 1)
set(WANT_OPENGL 1)
set(WANT_GLAD 1)
set(WANT_DBUS 1)
set(WANT_ALSA 1)
set(WANT_PULSEAUDIO 1)
set(WANT_X11 1)
set(WANT_FONTCONFIG 1)

add_compile_definitions(UNIX_ENABLED)
