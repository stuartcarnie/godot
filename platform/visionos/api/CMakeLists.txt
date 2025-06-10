godot_add_library(platform-visionos-api FOLDER visionos)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(platform-visionos-api PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(platform-visionos-api PRIVATE
        api.cpp
        api.h
)
# [[[end]]]
target_link_libraries(platform-visionos-api PRIVATE core)
