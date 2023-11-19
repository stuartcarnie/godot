godot_add_library(driver-png FOLDER drivers STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(driver-png PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(driver-png PRIVATE
        image_loader_png.cpp
        image_loader_png.h
        png_driver_common.cpp
        png_driver_common.h
        resource_saver_png.cpp
        resource_saver_png.h
)
# [[[end]]]

target_link_libraries(driver-png PUBLIC godot-io png)
