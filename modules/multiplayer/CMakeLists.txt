godot_add_library(godot-module-multiplayer FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-multiplayer PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-multiplayer PRIVATE
        multiplayer_debugger.cpp
        multiplayer_debugger.h
        multiplayer_spawner.cpp
        multiplayer_spawner.h
        multiplayer_synchronizer.cpp
        multiplayer_synchronizer.h
        register_types.cpp
        register_types.h
        scene_cache_interface.cpp
        scene_cache_interface.h
        scene_multiplayer.cpp
        scene_multiplayer.h
        scene_replication_config.cpp
        scene_replication_config.h
        scene_replication_interface.cpp
        scene_replication_interface.h
        scene_rpc_interface.cpp
        scene_rpc_interface.h
)
# [[[end]]]

if (WANT_EDITOR)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(godot-module-multiplayer PRIVATE')
    #   for file in list_files(cog, 'editor'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(godot-module-multiplayer PRIVATE
            editor/editor_network_profiler.cpp
            editor/editor_network_profiler.h
            editor/multiplayer_editor_plugin.cpp
            editor/multiplayer_editor_plugin.h
            editor/replication_editor.cpp
            editor/replication_editor.h
    )
    # [[[end]]]
endif ()
target_link_libraries(godot-module-multiplayer PUBLIC godot-object zlib)
