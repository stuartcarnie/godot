godot_add_library(godot-scene-3d-xr FOLDER scene STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-scene-3d-xr PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-scene-3d-xr PRIVATE
        xr_body_modifier_3d.cpp
        xr_body_modifier_3d.h
        xr_face_modifier_3d.cpp
        xr_face_modifier_3d.h
        xr_hand_modifier_3d.cpp
        xr_hand_modifier_3d.h
        xr_nodes.cpp
        xr_nodes.h
)
# [[[end]]]
target_link_libraries(godot-scene-3d-xr PRIVATE godot-core)
