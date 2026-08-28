godot_add_library(godot-module-texture_streaming FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-texture_streaming PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-texture_streaming PRIVATE
        register_types.cpp
        register_types.h
        texture_streaming.cpp
        texture_streaming.h
)
# [[[end]]]
target_link_libraries(godot-module-texture_streaming PUBLIC core)
group_target_sources(godot-module-texture_streaming)
