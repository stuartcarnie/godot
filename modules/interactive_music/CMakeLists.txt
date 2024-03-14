godot_add_library(godot-module-interactive_music FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-interactive_music PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-interactive_music PRIVATE
        audio_stream_interactive.cpp
        audio_stream_interactive.h
        audio_stream_playlist.cpp
        audio_stream_playlist.h
        audio_stream_synchronized.cpp
        audio_stream_synchronized.h
        register_types.cpp
        register_types.h
)
# [[[end]]]

if (WANT_EDITOR)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(godot-module-interactive_music PRIVATE')
    #   for file in list_files(cog, 'editor'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(godot-module-interactive_music PRIVATE
            editor/audio_stream_interactive_editor_plugin.cpp
            editor/audio_stream_interactive_editor_plugin.h
    )
    # [[[end]]]
endif ()

target_link_libraries(godot-module-interactive_music PRIVATE godot-config)
