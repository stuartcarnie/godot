godot_add_library(godot-module-objectdb_profiler FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-objectdb_profiler PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-objectdb_profiler PRIVATE
        register_types.cpp
        register_types.h
        snapshot_collector.cpp
        snapshot_collector.h
)
# [[[end]]]

if (WANT_EDITOR)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(godot-module-objectdb_profiler PRIVATE')
    #   for file in list_files(cog, ['editor', 'editor/data_viewers']):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(godot-module-objectdb_profiler PRIVATE
            editor/data_viewers/class_view.cpp
            editor/data_viewers/class_view.h
            editor/data_viewers/node_view.cpp
            editor/data_viewers/node_view.h
            editor/data_viewers/object_view.cpp
            editor/data_viewers/object_view.h
            editor/data_viewers/refcounted_view.cpp
            editor/data_viewers/refcounted_view.h
            editor/data_viewers/shared_controls.cpp
            editor/data_viewers/shared_controls.h
            editor/data_viewers/snapshot_view.cpp
            editor/data_viewers/snapshot_view.h
            editor/data_viewers/summary_view.cpp
            editor/data_viewers/summary_view.h
            editor/objectdb_profiler_panel.cpp
            editor/objectdb_profiler_panel.h
            editor/objectdb_profiler_plugin.cpp
            editor/objectdb_profiler_plugin.h
            editor/snapshot_data.cpp
            editor/snapshot_data.h
    )
    # [[[end]]]
endif ()
target_link_libraries(godot-module-objectdb_profiler PUBLIC core)
group_target_sources(godot-module-objectdb_profiler)
