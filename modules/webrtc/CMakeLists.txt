godot_add_library(godot-module-webrtc FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-webrtc PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-webrtc PRIVATE
        register_types.cpp
        register_types.h
        webrtc_data_channel.cpp
        webrtc_data_channel.h
        webrtc_data_channel_extension.cpp
        webrtc_data_channel_extension.h
        webrtc_data_channel_js.cpp
        webrtc_data_channel_js.h
        webrtc_multiplayer_peer.cpp
        webrtc_multiplayer_peer.h
        webrtc_peer_connection.cpp
        webrtc_peer_connection.h
        webrtc_peer_connection_extension.cpp
        webrtc_peer_connection_extension.h
        webrtc_peer_connection_js.cpp
        webrtc_peer_connection_js.h
)
# [[[end]]]
target_link_libraries(godot-module-webrtc PUBLIC godot-object)
