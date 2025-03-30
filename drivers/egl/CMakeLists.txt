godot_add_library(egl FOLDER drivers STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(egl PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(egl PRIVATE
        egl_manager.cpp
        egl_manager.h
)
# [[[end]]]

target_compile_definitions(egl PUBLIC $<${WANT_GLAD}:EGL_ENABLED=1>)
target_link_libraries(egl PUBLIC core $<${WANT_GLAD}:glad>)

group_target_sources(egl)
