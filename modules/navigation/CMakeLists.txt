godot_add_library(godot-module-navigation FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-navigation PRIVATE')
#   for file in list_files(cog, ['.', '2d', '3d']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-navigation PRIVATE
        2d/godot_navigation_server_2d.cpp
        2d/godot_navigation_server_2d.h
        2d/nav_mesh_generator_2d.cpp
        2d/nav_mesh_generator_2d.h
        3d/godot_navigation_server_3d.cpp
        3d/godot_navigation_server_3d.h
        3d/nav_mesh_generator_3d.cpp
        3d/nav_mesh_generator_3d.h
        3d/navigation_mesh_generator.cpp
        3d/navigation_mesh_generator.h
        nav_agent.cpp
        nav_agent.h
        nav_base.h
        nav_link.cpp
        nav_link.h
        nav_map.cpp
        nav_map.h
        nav_obstacle.cpp
        nav_obstacle.h
        nav_region.cpp
        nav_region.h
        nav_rid.h
        nav_utils.h
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
target_link_libraries(godot-module-navigation PUBLIC godot-object rvo2 recast-navigation zlib clipper2)
