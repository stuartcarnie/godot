set(GLCONTEXT_DRIVER_SOURCE_DIR ${GODOT_SOURCE_ROOT_THIRDPARTY_DIR}/glad)
godot_add_library(driver-glcontext FOLDER drivers STATIC)
# TODO: angle_libs do not use this
target_sources(driver-glcontext PRIVATE
        ${GLCONTEXT_DRIVER_SOURCE_DIR}/gl.c
)
target_compile_definitions(driver-glcontext PUBLIC GLAD_ENABLED=1 EGL_ENABLED=1)
target_include_directories(driver-glcontext PUBLIC ${GLCONTEXT_DRIVER_SOURCE_DIR})