godot_add_library(godot-error FOLDER core STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-error PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-error PRIVATE
        error_list.cpp
        error_list.h
        error_macros.cpp
        error_macros.h
)
# [[[end]]]
target_link_libraries(godot-error PUBLIC godot-object)