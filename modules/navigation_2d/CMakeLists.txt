godot_add_library(godot-module-navigation-2d FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-navigation-2d PRIVATE')
#   for file in list_files(cog, ['.', '2d']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-navigation-2d PRIVATE
        2d/godot_navigation_server_2d.cpp
        2d/godot_navigation_server_2d.h
        2d/nav_base_iteration_2d.h
        2d/nav_map_builder_2d.cpp
        2d/nav_map_builder_2d.h
        2d/nav_map_iteration_2d.h
        2d/nav_mesh_generator_2d.cpp
        2d/nav_mesh_generator_2d.h
        2d/nav_mesh_queries_2d.cpp
        2d/nav_mesh_queries_2d.h
        2d/nav_region_builder_2d.cpp
        2d/nav_region_builder_2d.h
        2d/nav_region_iteration_2d.h
        nav_agent_2d.cpp
        nav_agent_2d.h
        nav_base_2d.h
        nav_link_2d.cpp
        nav_link_2d.h
        nav_map_2d.cpp
        nav_map_2d.h
        nav_obstacle_2d.cpp
        nav_obstacle_2d.h
        nav_region_2d.cpp
        nav_region_2d.h
        nav_rid_2d.h
        nav_utils_2d.h
        register_types.cpp
        register_types.h
        triangle2.cpp
        triangle2.h
)
# [[[end]]]

if (WANT_EDITOR)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(godot-module-navigation-2d PRIVATE')
    #   for file in list_files(cog, 'editor'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(godot-module-navigation-2d PRIVATE
            editor/navigation_link_2d_editor_plugin.cpp
            editor/navigation_link_2d_editor_plugin.h
            editor/navigation_obstacle_2d_editor_plugin.cpp
            editor/navigation_obstacle_2d_editor_plugin.h
            editor/navigation_region_2d_editor_plugin.cpp
            editor/navigation_region_2d_editor_plugin.h
    )
    # [[[end]]]
endif ()

target_link_libraries(godot-module-navigation-2d PUBLIC core rvo2)
group_target_sources(godot-module-navigation-2d)
