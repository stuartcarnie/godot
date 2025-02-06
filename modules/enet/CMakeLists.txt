godot_add_library(godot-module-enet FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-enet PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-enet PRIVATE
        enet_connection.cpp
        enet_connection.h
        enet_multiplayer_peer.cpp
        enet_multiplayer_peer.h
        enet_packet_peer.cpp
        enet_packet_peer.h
        register_types.cpp
        register_types.h
)
# [[[end]]]
set(ENET_GODOT_SRC ${GODOT_SOURCE_ROOT_THIRDPARTY_DIR}/enet/enet_godot.cpp)
target_sources(godot-module-enet PRIVATE ${ENET_GODOT_SRC})
set_source_files_properties(${ENET_GODOT_SRC} PROPERTIES COMPILE_OPTIONS $<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>)
target_link_libraries(godot-module-enet PUBLIC godot-object godot-io godot-crypto enet)
