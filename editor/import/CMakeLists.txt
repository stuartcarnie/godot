add_subdirectory(3d)

godot_add_library(godot-editor-import FOLDER import)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-editor-import PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-editor-import PRIVATE
        audio_stream_import_settings.cpp
        audio_stream_import_settings.h
        dynamic_font_import_settings.cpp
        dynamic_font_import_settings.h
        editor_import_plugin.cpp
        editor_import_plugin.h
        resource_importer_bitmask.cpp
        resource_importer_bitmask.h
        resource_importer_bmfont.cpp
        resource_importer_bmfont.h
        resource_importer_csv_translation.cpp
        resource_importer_csv_translation.h
        resource_importer_dynamic_font.cpp
        resource_importer_dynamic_font.h
        resource_importer_image.cpp
        resource_importer_image.h
        resource_importer_imagefont.cpp
        resource_importer_imagefont.h
        resource_importer_layered_texture.cpp
        resource_importer_layered_texture.h
        resource_importer_shader_file.cpp
        resource_importer_shader_file.h
        resource_importer_texture.cpp
        resource_importer_texture.h
        resource_importer_texture_atlas.cpp
        resource_importer_texture_atlas.h
        resource_importer_texture_settings.cpp
        resource_importer_texture_settings.h
        resource_importer_wav.cpp
        resource_importer_wav.h
)
# [[[end]]]
target_link_libraries(godot-editor-import PRIVATE godot-config)
