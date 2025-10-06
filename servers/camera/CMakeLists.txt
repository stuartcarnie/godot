# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(servers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(servers PRIVATE
        camera_feed.cpp
        camera_feed.h
        camera_server.cpp
        camera_server.h
)
# [[[end]]]
