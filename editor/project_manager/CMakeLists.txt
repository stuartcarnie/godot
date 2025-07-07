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
        $<$<AND:$<BOOL:${WANT_OPENGL}>,$<BOOL:OPENGL_FOUND}>>:GLES3_ENABLED=1>
        $<$<AND:$<BOOL:${WANT_VULKAN}>,$<BOOL:${VULKAN_FOUND}>>:VULKAN_ENABLED=1 RD_ENABLED=1>
        $<$<BOOL:${WANT_METAL}>:METAL_ENABLED=1 RD_ENABLED=1>
)
