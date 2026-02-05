# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(scene PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(scene PRIVATE
        runtime_node_select.cpp
        runtime_node_select.h
        scene_debugger.cpp
        scene_debugger.h
        scene_debugger_object.cpp
        scene_debugger_object.h
)
# [[[end]]]
