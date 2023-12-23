set(GODOT_CORE_IO_CA_CERTIFICATES_SOURCE ${GODOT_SOURCE_ROOT_THIRDPARTY_DIR}/certs/ca-certificates.crt)
set(GODOT_CORE_IO_CA_CERTIFICATES_DESTINATION ${CMAKE_BINARY_INCLUDE_DIR}/core/io/certs_compressed.gen.h)
generate_file(certs_header ${GODOT_CORE_IO_CA_CERTIFICATES_SOURCE} ${GODOT_CORE_IO_CA_CERTIFICATES_DESTINATION})

godot_add_library(godot-io FOLDER core STATIC ${GODOT_CORE_IO_CA_CERTIFICATES_DESTINATION})
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-io PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-io PRIVATE
        compression.cpp
        compression.h
        config_file.cpp
        config_file.h
        dir_access.cpp
        dir_access.h
        dtls_server.cpp
        dtls_server.h
        file_access.cpp
        file_access.h
        file_access_compressed.cpp
        file_access_compressed.h
        file_access_encrypted.cpp
        file_access_encrypted.h
        file_access_memory.cpp
        file_access_memory.h
        file_access_pack.cpp
        file_access_pack.h
        file_access_zip.cpp
        file_access_zip.h
        http_client.cpp
        http_client.h
        http_client_tcp.cpp
        http_client_tcp.h
        image.cpp
        image.h
        image_loader.cpp
        image_loader.h
        ip.cpp
        ip.h
        ip_address.cpp
        ip_address.h
        json.cpp
        json.h
        logger.cpp
        logger.h
        marshalls.cpp
        marshalls.h
        missing_resource.cpp
        missing_resource.h
        net_socket.cpp
        net_socket.h
        packed_data_container.cpp
        packed_data_container.h
        packet_peer.cpp
        packet_peer.h
        packet_peer_dtls.cpp
        packet_peer_dtls.h
        packet_peer_udp.cpp
        packet_peer_udp.h
        pck_packer.cpp
        pck_packer.h
        plist.cpp
        plist.h
        remote_filesystem_client.cpp
        remote_filesystem_client.h
        resource.cpp
        resource.h
        resource_format_binary.cpp
        resource_format_binary.h
        resource_importer.cpp
        resource_importer.h
        resource_loader.cpp
        resource_loader.h
        resource_saver.cpp
        resource_saver.h
        resource_uid.cpp
        resource_uid.h
        stream_peer.cpp
        stream_peer.h
        stream_peer_gzip.cpp
        stream_peer_gzip.h
        stream_peer_tcp.cpp
        stream_peer_tcp.h
        stream_peer_tls.cpp
        stream_peer_tls.h
        tcp_server.cpp
        tcp_server.h
        translation_loader_po.cpp
        translation_loader_po.h
        udp_server.cpp
        udp_server.h
        xml_parser.cpp
        xml_parser.h
        zip_io.cpp
        zip_io.h
)
# [[[end]]]
target_link_libraries(godot-io
        PUBLIC
        godot-object
        godot-templates
        godot-string
        zstd
        zlib
        $<$<BOOL:${ENABLE_MINIZIP}>:minizip>

        PRIVATE
        $<$<BOOL:${ENABLE_BROTLI}>:brotli>
)
target_compile_definitions(godot-io
        PUBLIC
        $<$<BOOL:${ENABLE_MINIZIP}>:MINIZIP_ENABLED=1>

        PRIVATE
        $<$<BOOL:${ENABLE_BROTLI}>:BROTLI_ENABLED>
)

target_include_directories(godot-io PUBLIC ${CMAKE_BINARY_INCLUDE_DIR})
