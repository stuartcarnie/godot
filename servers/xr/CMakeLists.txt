godot_add_library(godot-server-xr FOLDER servers STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-server-xr PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-server-xr PRIVATE
        xr_body_tracker.cpp
        xr_body_tracker.h
        xr_controller_tracker.cpp
        xr_controller_tracker.h
        xr_face_tracker.cpp
        xr_face_tracker.h
        xr_hand_tracker.cpp
        xr_hand_tracker.h
        xr_interface.cpp
        xr_interface.h
        xr_interface_extension.cpp
        xr_interface_extension.h
        xr_pose.cpp
        xr_pose.h
        xr_positional_tracker.cpp
        xr_positional_tracker.h
        xr_tracker.cpp
        xr_tracker.h
        xr_vrs.cpp
        xr_vrs.h
)
# [[[end]]]
target_link_libraries(godot-server-xr PUBLIC godot-math godot-object godot-os godot-server-renderer)
