# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(core PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(core PRIVATE
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
