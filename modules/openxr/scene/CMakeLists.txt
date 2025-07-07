# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-openxr PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-openxr PRIVATE
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
        openxr_render_model.cpp
        openxr_render_model.h
        openxr_render_model_manager.cpp
        openxr_render_model_manager.h
        openxr_visibility_mask.cpp
        openxr_visibility_mask.h
)
# [[[end]]]
