# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(servers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(servers PRIVATE
        movie_writer.cpp
        movie_writer.h
        movie_writer_pngwav.cpp
        movie_writer_pngwav.h
)
# [[[end]]]
