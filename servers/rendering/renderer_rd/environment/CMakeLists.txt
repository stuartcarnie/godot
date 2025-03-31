# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(servers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(servers PRIVATE
        fog.cpp
        fog.h
        gi.cpp
        gi.h
        sky.cpp
        sky.h
)
# [[[end]]]
