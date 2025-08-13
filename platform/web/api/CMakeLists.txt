godot_add_library(platform-web-api FOLDER web)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(platform-web-api PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(platform-web-api PRIVATE
        api.cpp
        api.h
)
# [[[end]]]
target_link_libraries(platform-web-api PRIVATE core)
