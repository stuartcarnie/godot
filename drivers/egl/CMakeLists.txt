godot_add_library(driver-egl_manager FOLDER drivers STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(driver-egl_manager PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(driver-egl_manager PRIVATE
        egl_manager.cpp
        egl_manager.h
)
# [[[end]]]

target_compile_definitions(driver-egl_manager PUBLIC EGL_ENABLED=1)
target_link_libraries(driver-egl_manager PUBLIC godot-error glad)