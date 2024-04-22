godot_add_library(godot-editor-export FOLDER export)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-editor-export PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-editor-export PRIVATE
        editor_export.cpp
        editor_export.h
        editor_export_platform.cpp
        editor_export_platform.h
        editor_export_platform_pc.cpp
        editor_export_platform_pc.h
        editor_export_plugin.cpp
        editor_export_plugin.h
        editor_export_preset.cpp
        editor_export_preset.h
        export_template_manager.cpp
        export_template_manager.h
        project_export.cpp
        project_export.h
)
# [[[end]]]
target_link_libraries(godot-editor-export PRIVATE godot-config)
