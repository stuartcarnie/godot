godot_add_library(godot-server-movie-writer FOLDER servers STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-server-movie-writer PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-server-movie-writer PRIVATE
        movie_writer.cpp
        movie_writer.h
        movie_writer_mjpeg.cpp
        movie_writer_mjpeg.h
        movie_writer_pngwav.cpp
        movie_writer_pngwav.h
)
# [[[end]]]
target_link_libraries(godot-server-movie-writer PUBLIC godot-templates)
