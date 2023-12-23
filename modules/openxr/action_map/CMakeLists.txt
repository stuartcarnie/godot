godot_add_library(godot-module-openxr-action_map FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-openxr-action_map PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-openxr-action_map PRIVATE
        openxr_action.cpp
        openxr_action.h
        openxr_action_map.cpp
        openxr_action_map.h
        openxr_action_set.cpp
        openxr_action_set.h
        openxr_interaction_profile.cpp
        openxr_interaction_profile.h
        openxr_interaction_profile_metadata.cpp
        openxr_interaction_profile_metadata.h
)
# [[[end]]]
target_link_libraries(godot-module-openxr-action_map PUBLIC godot-object openxr)