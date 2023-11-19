godot_add_library(godot-module-msdfgen FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-msdfgen PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-msdfgen PRIVATE
        register_types.cpp
        register_types.h
)
# [[[end]]]
target_link_libraries(godot-module-msdfgen PUBLIC godot-object msdfgen)
