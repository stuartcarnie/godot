godot_add_library(enet FOLDER thirdparty STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(enet PRIVATE')
#   for file in list_files(cog, ['.', 'enet']):
#       if file.name == 'godot.cpp': continue
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(enet PRIVATE
        callbacks.c
        compress.c
        enet/callbacks.h
        enet/enet.h
        enet/godot.h
        enet/godot_ext.h
        enet/list.h
        enet/protocol.h
        enet/time.h
        enet/types.h
        enet/utility.h
        host.c
        list.c
        packet.c
        peer.c
        protocol.c
)
# [[[end]]]

target_include_directories(enet PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(enet PUBLIC GODOT_ENET=1)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(enet ws2_32)
endif()
target_compile_options(enet PRIVATE $<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>)
