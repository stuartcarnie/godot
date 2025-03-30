# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-core PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-core PRIVATE
        error_list.cpp
        error_list.h
        error_macros.cpp
        error_macros.h
)
# [[[end]]]
