# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(drivers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(drivers PRIVATE
        SDL_build_config_private.h
        joypad_sdl.cpp
        joypad_sdl.h
)
# [[[end]]]

target_compile_definitions(drivers
        PRIVATE
        SDL_ENABLED
)
target_link_libraries(drivers PUBLIC core sdl3)
