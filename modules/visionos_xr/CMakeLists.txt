godot_add_library(godot-module-visionos_xr FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-visionos_xr PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-visionos_xr PRIVATE
        register_types.h
        register_types.mm
        visionos_controller_tracking.h
        visionos_controller_tracking.mm
        visionos_definitions.h
        visionos_hand_tracking.h
        visionos_hand_tracking.mm
        visionos_simd_helpers.h
        visionos_xr_interface.h
        visionos_xr_interface.mm
)
# [[[end]]]

target_link_libraries(godot-module-visionos_xr PUBLIC core)
group_target_sources(godot-module-visionos_xr)
