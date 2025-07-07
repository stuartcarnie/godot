godot_add_library(godot-module-openxr FOLDER modules STATIC)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-openxr PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-openxr PRIVATE
        openxr_api.cpp
        openxr_api.h
        openxr_api_extension.cpp
        openxr_api_extension.h
        openxr_interface.cpp
        openxr_interface.h
        openxr_platform_inc.h
        openxr_util.cpp
        openxr_util.h
        openxr_uuid.h
        register_types.cpp
        register_types.h
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
            editor/openxr_binding_modifier_editor.cpp
            editor/openxr_binding_modifier_editor.h
            editor/openxr_binding_modifiers_dialog.cpp
            editor/openxr_binding_modifiers_dialog.h
            editor/openxr_editor_plugin.cpp
            editor/openxr_editor_plugin.h
            editor/openxr_interaction_profile_editor.cpp
            editor/openxr_interaction_profile_editor.h
            editor/openxr_select_action_dialog.cpp
            editor/openxr_select_action_dialog.h
            editor/openxr_select_interaction_profile_dialog.cpp
            editor/openxr_select_interaction_profile_dialog.h
            editor/openxr_select_runtime.cpp
            editor/openxr_select_runtime.h
    )
    # [[[end]]]
endif ()

add_subdirectory(action_map)
add_subdirectory(extensions)
add_subdirectory(scene)

target_sources(godot-module-openxr
        PRIVATE
        $<$<AND:$<BOOL:${WANT_OPENGL}>,$<BOOL:${OPENGL_FOUND}>,$<NOT:$<STREQUAL:${CMAKE_SYSTEM_NAME},Darwin>>>:extensions/platform/openxr_opengl_extension.cpp>
        $<$<AND:$<BOOL:${WANT_VULKAN}>,$<BOOL:${VULKAN_FOUND}>>:extensions/platform/openxr_vulkan_extension.cpp>
        $<$<BOOL:${WANT_METAL}>:extensions/platform/openxr_metal_extension.mm>
)

target_link_libraries(godot-module-openxr
        PUBLIC
        $<$<AND:$<BOOL:${WANT_OPENGL}>,$<BOOL:${OPENGL_FOUND}>,$<NOT:$<STREQUAL:${CMAKE_SYSTEM_NAME},Darwin>>>:gles3>
        $<$<AND:$<BOOL:${WANT_VULKAN}>,$<BOOL:${VULKAN_FOUND}>>:driver-vulkan>
        $<$<BOOL:${WANT_METAL}>:metal>
)

target_link_libraries(godot-module-openxr PUBLIC core openxr servers zlib)
set_property(TARGET godot-module-openxr PROPERTY CXX_STANDARD 17)

group_target_sources(godot-module-openxr)
