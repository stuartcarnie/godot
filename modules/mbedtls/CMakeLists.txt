godot_add_library(godot-module-mbedtls FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-mbedtls PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-mbedtls PRIVATE
        crypto_mbedtls.cpp
        crypto_mbedtls.h
        dtls_server_mbedtls.cpp
        dtls_server_mbedtls.h
        packet_peer_mbed_dtls.cpp
        packet_peer_mbed_dtls.h
        register_types.cpp
        register_types.h
        stream_peer_mbedtls.cpp
        stream_peer_mbedtls.h
        tls_context_mbedtls.cpp
        tls_context_mbedtls.h
)
# [[[end]]]
target_link_libraries(godot-module-mbedtls PUBLIC godot-object mbedtls godot-io)
