# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(servers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(servers PRIVATE
        accessibility_server.cpp
        accessibility_server.h
        accessibility_server_dummy.h
        accessibility_server_enums.h
        display_server.cpp
        display_server.h
        display_server_enums.h
        display_server_headless.cpp
        display_server_headless.h
        native_menu.cpp
        native_menu.h
)
# [[[end]]]
