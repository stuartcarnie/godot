godot_add_library(godot-module-meshoptimizer FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-meshoptimizer PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-meshoptimizer PRIVATE
        register_types.cpp
        register_types.h
)
# [[[end]]]
target_link_libraries(godot-module-meshoptimizer PUBLIC godot-object meshoptimizer)
