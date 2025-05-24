# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        codesign.cpp
        codesign.h
        editor_export.cpp
        editor_export.h
        editor_export_platform.cpp
        editor_export_platform.h
        editor_export_platform_apple_embedded.cpp
        editor_export_platform_apple_embedded.h
        editor_export_platform_extension.cpp
        editor_export_platform_extension.h
        editor_export_platform_pc.cpp
        editor_export_platform_pc.h
        editor_export_plugin.cpp
        editor_export_plugin.h
        editor_export_preset.cpp
        editor_export_preset.h
        export_template_manager.cpp
        export_template_manager.h
        lipo.cpp
        lipo.h
        macho.cpp
        macho.h
        plugin_config_apple_embedded.cpp
        plugin_config_apple_embedded.h
        project_export.cpp
        project_export.h
        project_zip_packer.cpp
        project_zip_packer.h
)
# [[[end]]]
