godot_add_library(wslay FOLDER thirdparty STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(wslay PRIVATE')
#   for file in list_files(cog, ['.', 'wslay']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(wslay PRIVATE
        config.h
        wslay/wslay.h
        wslay/wslayver.h
        wslay_event.c
        wslay_event.h
        wslay_frame.c
        wslay_frame.h
        wslay_macro.h
        wslay_net.c
        wslay_net.h
        wslay_queue.c
        wslay_queue.h
)
# [[[end]]]
target_include_directories(wslay PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(wslay PRIVATE -DHAVE_CONFIG_H=1)
target_compile_options(wslay PRIVATE $<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>)
