set(_OpenGLES3_REQUIRED_VARS OPENGLES3_gles_LIBRARY)

set(_OpenGLES3_CACHE_VARS)

if (APPLE)
    find_library(OPENGLES3_gles_LIBRARY OpenGLES DOC "OpenGLES library for iOS")
    find_path(OPENGLES3_INCLUDE_DIR ES3/gl.h DOC "Include for OpenGL on OS X")
    list(APPEND _OpenGLES3_REQUIRED_VARS OPENGLES3_INCLUDE_DIR)

    list(APPEND _OpenGLES3_CACHE_VARS
            OPENGLES3_INCLUDE_DIR
            OPENGLES3_gles_LIBRARY
    )
else ()
    message(FATAL_ERROR "OpenGLES3 is not supported on this platform")
endif ()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OpenGLES3 REQUIRED_VARS ${_OpenGLES3_REQUIRED_VARS}
        HANDLE_COMPONENTS)
unset(_OpenGLES3_REQUIRED_VARS)

# OpenGL:: targets
if(OPENGLES3_FOUND)
    if(OPENGLES3_gles_LIBRARY AND NOT TARGET OpenGLES3::ES3)
        # A legacy GL library is available, so use it for the legacy GL target.
        if(IS_ABSOLUTE "${OPENGLES3_gles_LIBRARY}")
            add_library(OpenGLES3::ES3 UNKNOWN IMPORTED)
            set_target_properties(OpenGLES3::ES3 PROPERTIES
                    IMPORTED_LOCATION "${OPENGLES3_gles_LIBRARY}")
        else()
            add_library(OpenGLES3::ES3 INTERFACE IMPORTED)
            set_target_properties(OpenGLES3::ES3 PROPERTIES
                    IMPORTED_LIBNAME "${OPENGLES3_gles_LIBRARY}")
        endif()
        target_compile_definitions(OpenGLES3::ES3 INTERFACE
                GLES_SILENCE_DEPRECATION)
        set_target_properties(OpenGLES3::ES3 PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${OPENGLES3_INCLUDE_DIR}")
    endif()

    # OPENGLES3_LIBRARIES mirrors OpenGLES3::ES3 logic ...
    if(OPENGLES3_gles_LIBRARY)
        set(OPENGLES3_LIBRARIES ${OPENGLES3_gles_LIBRARY})
    else()
        set(OPENGL_LIBRARIES "")
    endif()
endif ()

mark_as_advanced(${_OpenGLES3_CACHE_VARS})
unset(_OpenGLES3_CACHE_VARS)
