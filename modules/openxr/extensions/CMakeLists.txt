godot_add_library(godot-module-openxr-extensions FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-openxr-extensions PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-openxr-extensions PRIVATE
        openxr_composition_layer_depth_extension.cpp
        openxr_composition_layer_depth_extension.h
        openxr_composition_layer_extension.cpp
        openxr_composition_layer_extension.h
        openxr_composition_layer_provider.h
        openxr_extension_wrapper.h
        openxr_extension_wrapper_extension.cpp
        openxr_extension_wrapper_extension.h
        openxr_eye_gaze_interaction.cpp
        openxr_eye_gaze_interaction.h
        openxr_fb_display_refresh_rate_extension.cpp
        openxr_fb_display_refresh_rate_extension.h
        openxr_fb_foveation_extension.cpp
        openxr_fb_foveation_extension.h
        openxr_fb_update_swapchain_extension.cpp
        openxr_fb_update_swapchain_extension.h
        openxr_hand_interaction_extension.cpp
        openxr_hand_interaction_extension.h
        openxr_hand_tracking_extension.cpp
        openxr_hand_tracking_extension.h
        openxr_htc_controller_extension.cpp
        openxr_htc_controller_extension.h
        openxr_htc_vive_tracker_extension.cpp
        openxr_htc_vive_tracker_extension.h
        openxr_huawei_controller_extension.cpp
        openxr_huawei_controller_extension.h
        openxr_local_floor_extension.cpp
        openxr_local_floor_extension.h
        openxr_meta_controller_extension.cpp
        openxr_meta_controller_extension.h
        openxr_ml2_controller_extension.cpp
        openxr_ml2_controller_extension.h
        openxr_palm_pose_extension.cpp
        openxr_palm_pose_extension.h
        openxr_pico_controller_extension.cpp
        openxr_pico_controller_extension.h
        openxr_wmr_controller_extension.cpp
        openxr_wmr_controller_extension.h
)
# [[[end]]]

if (${WANT_OPENGL} AND ${OPENGL_FOUND} AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    target_sources(godot-module-openxr-extensions PRIVATE platform/openxr_opengl_extension.cpp)
endif ()
if (${WANT_VULKAN} AND ${VULKAN_FOUND})
    target_sources(godot-module-openxr-extensions PRIVATE platform/openxr_vulkan_extension.cpp)
endif ()
if (${WANT_OPENGL} AND ${OPENGL_FOUND})
    target_link_libraries(godot-module-openxr-extensions PUBLIC driver-gles3)
endif ()
if (${WANT_VULKAN} AND ${VULKAN_FOUND})
    target_link_libraries(godot-module-openxr-extensions PUBLIC driver-vulkan)
endif ()

target_link_libraries(godot-module-openxr-extensions PUBLIC godot-object openxr)
