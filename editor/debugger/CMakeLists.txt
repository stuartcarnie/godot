add_subdirectory(debug_adapter)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(editor PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(editor PRIVATE
        debugger_editor_plugin.cpp
        debugger_editor_plugin.h
        editor_debugger_inspector.cpp
        editor_debugger_inspector.h
        editor_debugger_node.cpp
        editor_debugger_node.h
        editor_debugger_plugin.cpp
        editor_debugger_plugin.h
        editor_debugger_server.cpp
        editor_debugger_server.h
        editor_debugger_tree.cpp
        editor_debugger_tree.h
        editor_expression_evaluator.cpp
        editor_expression_evaluator.h
        editor_file_server.cpp
        editor_file_server.h
        editor_performance_profiler.cpp
        editor_performance_profiler.h
        editor_profiler.cpp
        editor_profiler.h
        editor_visual_profiler.cpp
        editor_visual_profiler.h
        script_editor_debugger.cpp
        script_editor_debugger.h
)
# [[[end]]]
