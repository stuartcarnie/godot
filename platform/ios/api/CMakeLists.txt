godot_add_library(platform-ios-api FOLDER ios)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(platform-ios-api PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(platform-ios-api PRIVATE
        api.cpp
        api.h
)
# [[[end]]]
target_link_libraries(platform-ios-api PRIVATE godot-config)
