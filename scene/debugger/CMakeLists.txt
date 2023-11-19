godot_add_library(godot-scene-debugger FOLDER scene STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-scene-debugger PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-scene-debugger PRIVATE
        scene_debugger.cpp
        scene_debugger.h
)
# [[[end]]]
target_link_libraries(godot-scene-debugger PUBLIC godot-object)
