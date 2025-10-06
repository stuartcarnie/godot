# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(servers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(servers PRIVATE
        text_server.cpp
        text_server.h
        text_server_dummy.h
        text_server_extension.cpp
        text_server_extension.h
)
# [[[end]]]
