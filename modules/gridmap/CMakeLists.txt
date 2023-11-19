godot_add_library(godot-module-gridmap FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-gridmap PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-gridmap PRIVATE
        grid_map.cpp
        grid_map.h
        register_types.cpp
        register_types.h
)
# [[[end]]]

if (WANT_EDITOR)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(godot-module-gridmap PRIVATE')
    #   for file in list_files(cog, 'editor'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(godot-module-gridmap PRIVATE
            editor/grid_map_editor_plugin.cpp
            editor/grid_map_editor_plugin.h
    )
    # [[[end]]]
endif ()
target_link_libraries(godot-module-gridmap PUBLIC godot-object minizip)
