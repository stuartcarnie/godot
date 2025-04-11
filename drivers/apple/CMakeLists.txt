# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(drivers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(drivers PRIVATE
        joypad_apple.h
        joypad_apple.mm
        thread_apple.cpp
        thread_apple.h
)
# [[[end]]]

