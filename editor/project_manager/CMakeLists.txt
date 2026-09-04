# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        engine_update_label.cpp
        engine_update_label.h
        project_dialog.cpp
        project_dialog.h
        project_list.cpp
        project_list.h
        project_manager.cpp
        project_manager.h
        project_tag.cpp
        project_tag.h
        quick_settings_dialog.cpp
        quick_settings_dialog.h
)
# [[[end]]]
target_compile_definitions(editor
        PRIVATE
        $<$<BOOL:${ENABLE_RENDERING_DEVICE}>:RD_ENABLED> $<$<BOOL:${ENABLE_FORWARD_PLUS_RENDERER}>:FORWARD_RD_ENABLED> $<$<BOOL:${ENABLE_MOBILE_RENDERER}>:MOBILE_RD_ENABLED>
        $<$<AND:$<BOOL:${ENABLE_OPENGL}>,$<BOOL:${OPENGL_FOUND}>>:GLES3_ENABLED>
        $<$<AND:$<BOOL:${ENABLE_VULKAN}>,$<BOOL:${VULKAN_FOUND}>>:VULKAN_ENABLED>
        $<$<BOOL:${ENABLE_METAL}>:METAL_ENABLED>
)
