godot_add_library(godot-module-jpg FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-jpg PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-jpg PRIVATE
        image_loader_libjpeg_turbo.cpp
        image_loader_libjpeg_turbo.h
        movie_writer_mjpeg.cpp
        movie_writer_mjpeg.h
        register_types.cpp
        register_types.h
)
# [[[end]]]
target_link_libraries(godot-module-jpg PUBLIC core jpeg-turbo)
group_target_sources(godot-module-jpg)
