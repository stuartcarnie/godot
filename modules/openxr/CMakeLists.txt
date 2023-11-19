godot_add_library(godot-module-openxr FOLDER modules STATIC)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-openxr PRIVATE')
#   for file in list_files(cog, ['.', 'action_map', 'scene']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-openxr PRIVATE
        action_map/openxr_action.cpp
        action_map/openxr_action.h
        action_map/openxr_action_map.cpp
        action_map/openxr_action_map.h
        action_map/openxr_action_set.cpp
        action_map/openxr_action_set.h
        action_map/openxr_interaction_profile.cpp
        action_map/openxr_interaction_profile.h
        action_map/openxr_interaction_profile_metadata.cpp
        action_map/openxr_interaction_profile_metadata.h
        openxr_api.cpp
        openxr_api.h
        openxr_api_extension.cpp
        openxr_api_extension.h
        openxr_interface.cpp
        openxr_interface.h
        openxr_platform_inc.h
        openxr_util.cpp
        openxr_util.h
        register_types.cpp
        register_types.h
        scene/openxr_hand.cpp
        scene/openxr_hand.h
        util.h
)
# [[[end]]]

if (WANT_EDITOR)
	# [[[cog
	#   import cog
	#   from cmake_builders import list_files
	#   cog.outl('target_sources(godot-module-openxr PRIVATE')
	#   for file in list_files(cog, 'editor'):
	#       cog.outl(f'        {file}')
	#   cog.outl(')')
	# ]]]
	target_sources(godot-module-openxr PRIVATE
	        editor/openxr_action_editor.cpp
	        editor/openxr_action_editor.h
	        editor/openxr_action_map_editor.cpp
	        editor/openxr_action_map_editor.h
	        editor/openxr_action_set_editor.cpp
	        editor/openxr_action_set_editor.h
	        editor/openxr_editor_plugin.cpp
	        editor/openxr_editor_plugin.h
	        editor/openxr_interaction_profile_editor.cpp
	        editor/openxr_interaction_profile_editor.h
	        editor/openxr_select_action_dialog.cpp
	        editor/openxr_select_action_dialog.h
	        editor/openxr_select_interaction_profile_dialog.cpp
	        editor/openxr_select_interaction_profile_dialog.h
	)
	# [[[end]]]
endif ()

target_sources(godot-module-openxr PRIVATE
		$<${WANT_VULKAN}:extensions/openxr_vulkan_extension.cpp>
		extensions/openxr_palm_pose_extension.cpp
		extensions/openxr_composition_layer_depth_extension.cpp
		extensions/openxr_eye_gaze_interaction.cpp
		extensions/openxr_htc_controller_extension.cpp
		extensions/openxr_htc_vive_tracker_extension.cpp
		extensions/openxr_huawei_controller_extension.cpp
		extensions/openxr_hand_tracking_extension.cpp
		extensions/openxr_fb_foveation_extension.cpp
		extensions/openxr_fb_update_swapchain_extension.cpp
		extensions/openxr_fb_passthrough_extension_wrapper.cpp
		extensions/openxr_fb_display_refresh_rate_extension.cpp
		extensions/openxr_pico_controller_extension.cpp
		extensions/openxr_wmr_controller_extension.cpp
		extensions/openxr_ml2_controller_extension.cpp
		extensions/openxr_extension_wrapper_extension.cpp
)

if(${WANT_OPENGL} AND ${OPENGL_FOUND} AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	target_sources(godot-module-openxr PRIVATE extensions/openxr_opengl_extension.cpp)
endif()
if(${WANT_VULKAN} AND ${VULKAN_FOUND})
	target_sources(godot-module-openxr PRIVATE extensions/openxr_vulkan_extension.cpp)
endif()

target_link_libraries(godot-module-openxr PUBLIC godot-object godot-core openxr godot-server-renderer zlib)
set_property(TARGET godot-module-openxr PROPERTY CXX_STANDARD 17)
if(${WANT_OPENGL} AND ${OPENGL_FOUND})
	target_link_libraries(godot-module-openxr PUBLIC driver-gles3)
endif()
if(${WANT_VULKAN} AND ${VULKAN_FOUND})
	target_link_libraries(godot-module-openxr PUBLIC driver-vulkan)
endif()

if(${WANT_X11})
	target_compile_definitions(godot-module-openxr PUBLIC XR_USE_PLATFORM_XLIB=1)
endif()

target_compile_definitions(godot-module-openxr PUBLIC
		DISABLE_STD_FILESYSTEM
		XRLOADER_DISABLE_EXCEPTION_HANDLING
		JSON_USE_EXCEPTION=0)