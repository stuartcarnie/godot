godot_add_library(driver-apple FOLDER drivers STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(driver-apple PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(driver-apple PRIVATE
        joypad_apple.h
        joypad_apple.mm
)
# [[[end]]]

target_link_libraries(driver-apple PUBLIC godot-error)

group_target_sources(driver-apple)
