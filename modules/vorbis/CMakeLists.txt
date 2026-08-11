godot_add_library(godot-module-vorbis FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-vorbis PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-vorbis PRIVATE
        audio_stream_ogg_vorbis.cpp
        audio_stream_ogg_vorbis.h
        register_types.cpp
        register_types.h
        resource_importer_ogg_vorbis.cpp
        resource_importer_ogg_vorbis.h
)
# [[[end]]]

if (WANT_EDITOR)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(godot-module-vorbis PRIVATE')
    #   for file in list_files(cog, 'editor'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(godot-module-vorbis PRIVATE
            editor/audio_stream_ogg_vorbis_editor_plugin.cpp
            editor/audio_stream_ogg_vorbis_editor_plugin.h
    )
    # [[[end]]]
endif ()

target_link_libraries(godot-module-vorbis PUBLIC core vorbis)
group_target_sources(godot-module-vorbis)
