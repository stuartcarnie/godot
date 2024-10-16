godot_add_library(platform-android-api FOLDER android)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(platform-android-api PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(platform-android-api PRIVATE
        api.cpp
        api.h
)
# [[[end]]]
target_link_libraries(platform-android-api PRIVATE godot-config)
