godot_add_library(miniupnpc FOLDER thirdparty STATIC)
target_sources(miniupnpc PRIVATE
        include/miniupnpc/upnpcommands.h
        include/miniupnpc/miniupnpc_declspec.h
        include/miniupnpc/miniwget.h
        include/miniupnpc/portlistingparse.h
        include/miniupnpc/upnpreplyparse.h
        include/miniupnpc/miniupnpctypes.h
        include/miniupnpc/igd_desc_parse.h
        include/miniupnpc/upnpdev.h
        include/miniupnpc/miniupnpc.h
        src/miniupnpcstrings.h
        src/minisoap.h
        src/minissdpc.c
        src/addr_is_reserved.c
        src/minissdpc.h
        src/minixml.c
        #src/minixmlvalid.c
        src/miniupnpc_socketdef.h
        src/miniwget_private.h
        src/connecthostport.h
        src/addr_is_reserved.h
        src/upnpcommands.c
        src/portlistingparse.c
        src/igd_desc_parse.c
        src/minisoap.c
        src/receivedata.c
        src/upnpreplyparse.c
        src/codelength.h
        src/win32_snprintf.h
        src/upnpdev.c
        src/minixml.h
        src/miniwget.c
        src/receivedata.h
        src/miniupnpc.c
        src/connecthostport.c
)
target_link_libraries(miniupnpc godot-core)
target_include_directories(miniupnpc
        PUBLIC include
        PRIVATE include/miniupnpc
)
target_compile_definitions(miniupnpc PUBLIC MINIUPNP_STATICLIB MINIUPNPC_SET_SOCKET_TIMEOUT)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(miniupnpc iphlpapi)
endif()
