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
        openxr_composition_layer.cpp
        openxr_composition_layer.h
        openxr_composition_layer_cylinder.cpp
        openxr_composition_layer_cylinder.h
        openxr_composition_layer_equirect.cpp
        openxr_composition_layer_equirect.h
        openxr_composition_layer_quad.cpp
        openxr_composition_layer_quad.h
        openxr_hand.cpp
        openxr_hand.h
)
# [[[end]]]
target_link_libraries(godot-module-openxr-scene PUBLIC godot-object openxr)
