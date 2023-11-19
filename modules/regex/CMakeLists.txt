godot_add_library(godot-module-regex FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-regex PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-regex PRIVATE
        regex.cpp
        regex.h
        register_types.cpp
        register_types.h
)
# [[[end]]]
target_link_libraries(godot-module-regex PUBLIC godot-object pcre2)
