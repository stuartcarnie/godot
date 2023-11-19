godot_add_library(godot-module-websocket FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-websocket PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-websocket PRIVATE
        emws_peer.cpp
        emws_peer.h
        packet_buffer.h
        register_types.cpp
        register_types.h
        remote_debugger_peer_websocket.cpp
        remote_debugger_peer_websocket.h
        websocket_multiplayer_peer.cpp
        websocket_multiplayer_peer.h
        websocket_peer.cpp
        websocket_peer.h
        wsl_peer.cpp
        wsl_peer.h
)
# [[[end]]]

if (WANT_EDITOR)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(godot-module-websocket PRIVATE')
    #   for file in list_files(cog, 'editor'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(godot-module-websocket PRIVATE
            editor/editor_debugger_server_websocket.cpp
            editor/editor_debugger_server_websocket.h
    )
    # [[[end]]]
endif ()
target_link_libraries(godot-module-websocket PUBLIC godot-object wslay zlib)
set_target_properties(godot-module-websocket PROPERTIES
        CXX_STANDARD            17
        CXX_STANDARD_REQUIRED   ON
)
