if (${WANT_OPENGL} AND ${OPENGL_FOUND})
    godot_add_library(glad FOLDER thirdparty STATIC)
    target_sources(glad PRIVATE
            glad/glx.h
            glad/gl.h
            gl.c
            KHR/khrplatform.h
    )
    if (CMAKE_SYSTEM_NAME STREQUAL "Windows" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "iOS")
        target_sources(glad PRIVATE
                # TODO: if using angle_libs, don't add this
                egl.c
        )
    else ()
        target_sources(glad PRIVATE
                glx.c
        )
    endif ()
    target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endif ()
