#set(GLCONTEXT_DRIVER_SOURCE_DIR ${GODOT_SOURCE_ROOT_THIRDPARTY_DIR}/glad)
#godot_add_library(gl_context FOLDER drivers STATIC)
## TODO: angle_libs do not use this
#target_sources(gl_context PRIVATE
#        ${GLCONTEXT_DRIVER_SOURCE_DIR}/gl.c
#)
#target_compile_definitions(gl_context PUBLIC GLAD_ENABLED $<$<PLATFORM_ID:Linux>:EGL_ENABLED GLAD_GLES2>)
#target_include_directories(gl_context PUBLIC ${GLCONTEXT_DRIVER_SOURCE_DIR})
#
#group_target_sources(gl_context)
