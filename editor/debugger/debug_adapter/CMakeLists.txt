godot_add_library(godot-editor-debugger-debug_adapter FOLDER debugger)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-editor-debugger-debug_adapter PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-editor-debugger-debug_adapter PRIVATE
        debug_adapter_parser.cpp
        debug_adapter_parser.h
        debug_adapter_protocol.cpp
        debug_adapter_protocol.h
        debug_adapter_server.cpp
        debug_adapter_server.h
        debug_adapter_types.h
)
# [[[end]]]
target_link_libraries(godot-editor-debugger-debug_adapter PRIVATE godot-config)
