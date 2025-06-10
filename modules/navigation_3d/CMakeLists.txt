godot_add_library(godot-module-navigation-3d FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-navigation-3d PRIVATE')
#   for file in list_files(cog, ['.', '3d']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-navigation-3d PRIVATE
        3d/godot_navigation_server_3d.cpp
        3d/godot_navigation_server_3d.h
        3d/nav_base_iteration_3d.h
        3d/nav_map_builder_3d.cpp
        3d/nav_map_builder_3d.h
        3d/nav_map_iteration_3d.h
        3d/nav_mesh_generator_3d.cpp
        3d/nav_mesh_generator_3d.h
        3d/nav_mesh_queries_3d.cpp
        3d/nav_mesh_queries_3d.h
        3d/nav_region_builder_3d.cpp
        3d/nav_region_builder_3d.h
        3d/nav_region_iteration_3d.h
        3d/navigation_mesh_generator.cpp
        3d/navigation_mesh_generator.h
        nav_agent_3d.cpp
        nav_agent_3d.h
        nav_base_3d.h
        nav_link_3d.cpp
        nav_link_3d.h
        nav_map_3d.cpp
        nav_map_3d.h
        nav_obstacle_3d.cpp
        nav_obstacle_3d.h
        nav_region_3d.cpp
        nav_region_3d.h
        nav_rid_3d.h
        nav_utils_3d.h
        register_types.cpp
        register_types.h
)
# [[[end]]]

if (WANT_EDITOR)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(godot-module-navigation-3d PRIVATE')
    #   for file in list_files(cog, 'editor'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(godot-module-navigation-3d PRIVATE
            editor/navigation_link_3d_editor_plugin.cpp
            editor/navigation_link_3d_editor_plugin.h
            editor/navigation_link_3d_gizmo_plugin.cpp
            editor/navigation_link_3d_gizmo_plugin.h
            editor/navigation_obstacle_3d_editor_plugin.cpp
            editor/navigation_obstacle_3d_editor_plugin.h
            editor/navigation_region_3d_editor_plugin.cpp
            editor/navigation_region_3d_editor_plugin.h
            editor/navigation_region_3d_gizmo_plugin.cpp
            editor/navigation_region_3d_gizmo_plugin.h
    )
    # [[[end]]]
endif ()
target_link_libraries(godot-module-navigation-3d PUBLIC core rvo2 recast-navigation zlib clipper2)
group_target_sources(godot-module-navigation-3d)
