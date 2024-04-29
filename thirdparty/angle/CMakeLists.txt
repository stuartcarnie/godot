godot_add_library(angle FOLDER thirdparty NO_INCLUDES INTERFACE
        include/EGL/egl.h
        include/EGL/eglext.h
        include/EGL/eglext_angle.h
        include/EGL/eglplatform.h
        include/KHR/khrplatform.h
)
target_include_directories(angle INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
