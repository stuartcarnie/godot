godot_add_library(godot-module-spirv-cross FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-spirv-cross PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-spirv-cross PRIVATE
        register_types.cpp
        register_types.h
)
# [[[end]]]
