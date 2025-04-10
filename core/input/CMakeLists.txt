# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(core PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(core PRIVATE
        default_controller_mappings.h
        input.cpp
        input.h
        input_enums.h
        input_event.cpp
        input_event.h
        input_map.cpp
        input_map.h
        shortcut.cpp
        shortcut.h
)
# [[[end]]]
