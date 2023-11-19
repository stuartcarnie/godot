godot_add_library(godot-module-navigation FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-navigation PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-navigation PRIVATE
        godot_navigation_server.cpp
        godot_navigation_server.h
        godot_navigation_server_2d.cpp
        godot_navigation_server_2d.h
        nav_agent.cpp
        nav_agent.h
        nav_base.h
        nav_link.cpp
        nav_link.h
        nav_map.cpp
        nav_map.h
        nav_mesh_generator_2d.cpp
        nav_mesh_generator_2d.h
        nav_mesh_generator_3d.cpp
        nav_mesh_generator_3d.h
        nav_obstacle.cpp
        nav_obstacle.h
        nav_region.cpp
        nav_region.h
        nav_rid.h
        nav_utils.h
        navigation_mesh_generator.cpp
        navigation_mesh_generator.h
        register_types.cpp
        register_types.h
)
# [[[end]]]

if (WANT_EDITOR)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(godot-module-navigation PRIVATE')
    #   for file in list_files(cog, 'editor'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(godot-module-navigation PRIVATE
            editor/navigation_mesh_editor_plugin.cpp
            editor/navigation_mesh_editor_plugin.h
    )
    # [[[end]]]
endif ()
target_link_libraries(godot-module-navigation PUBLIC godot-object rvo2 recast-navigation zlib)
