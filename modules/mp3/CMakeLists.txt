godot_add_library(godot-module-mp3 FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-mp3 PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-mp3 PRIVATE
        audio_stream_mp3.cpp
        audio_stream_mp3.h
        register_types.cpp
        register_types.h
        resource_importer_mp3.cpp
        resource_importer_mp3.h
)
# [[[end]]]

if (WANT_EDITOR)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(godot-module-mp3 PRIVATE')
    #   for file in list_files(cog, 'editor'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(godot-module-mp3 PRIVATE
            editor/audio_stream_mp3_editor_plugin.cpp
            editor/audio_stream_mp3_editor_plugin.h
    )
    # [[[end]]]
endif ()

target_link_libraries(godot-module-mp3 PUBLIC core)
group_target_sources(godot-module-mp3)
