# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        debug_adapter_parser.cpp
        debug_adapter_parser.h
        debug_adapter_protocol.cpp
        debug_adapter_protocol.h
        debug_adapter_server.cpp
        debug_adapter_server.h
        debug_adapter_types.h
)
# [[[end]]]
