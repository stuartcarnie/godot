godot_add_library(driver-windows FOLDER drivers STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(driver-windows PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(driver-windows PRIVATE
        dir_access_windows.cpp
        dir_access_windows.h
        file_access_windows.cpp
        file_access_windows.h
        file_access_windows_pipe.cpp
        file_access_windows_pipe.h
)
# [[[end]]]
target_link_libraries(driver-windows PUBLIC godot-core)
