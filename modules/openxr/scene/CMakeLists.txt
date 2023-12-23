godot_add_library(godot-module-openxr-scene FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-openxr-scene PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-openxr-scene PRIVATE
        openxr_hand.cpp
        openxr_hand.h
)
# [[[end]]]
target_link_libraries(godot-module-openxr-scene PUBLIC godot-object openxr)