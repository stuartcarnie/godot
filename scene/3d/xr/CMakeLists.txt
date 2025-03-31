# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(scene PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(scene PRIVATE
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
