godot_add_library(godot-debugger FOLDER core STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-debugger PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-debugger PRIVATE
        debugger_marshalls.cpp
        debugger_marshalls.h
        engine_debugger.cpp
        engine_debugger.h
        engine_profiler.cpp
        engine_profiler.h
        local_debugger.cpp
        local_debugger.h
        remote_debugger.cpp
        remote_debugger.h
        remote_debugger_peer.cpp
        remote_debugger_peer.h
        script_debugger.cpp
        script_debugger.h
)
# [[[end]]]
target_link_libraries(godot-debugger PUBLIC godot-object)
