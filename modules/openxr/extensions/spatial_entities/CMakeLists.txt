# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-openxr PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-openxr PRIVATE
        openxr_spatial_anchor.cpp
        openxr_spatial_anchor.h
        openxr_spatial_entities.cpp
        openxr_spatial_entities.h
        openxr_spatial_entity_extension.cpp
        openxr_spatial_entity_extension.h
        openxr_spatial_marker_tracking.cpp
        openxr_spatial_marker_tracking.h
        openxr_spatial_plane_tracking.cpp
        openxr_spatial_plane_tracking.h
)
# [[[end]]]
