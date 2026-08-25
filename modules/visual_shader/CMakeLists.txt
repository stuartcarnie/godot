godot_add_library(godot-module-visual_shader FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-visual_shader PRIVATE')
#   for file in list_files(cog, ['.', 'vs_nodes']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-visual_shader PRIVATE
        register_types.cpp
        register_types.h
        shader_graph.cpp
        shader_graph.h
        visual_shader.cpp
        visual_shader.h
        visual_shader_group.cpp
        visual_shader_group.h
        vs_nodes/visual_shader_nodes.cpp
        vs_nodes/visual_shader_nodes.h
        vs_nodes/visual_shader_particle_nodes.cpp
        vs_nodes/visual_shader_particle_nodes.h
        vs_nodes/visual_shader_sdf_nodes.cpp
        vs_nodes/visual_shader_sdf_nodes.h
)
# [[[end]]]

if (WANT_EDITOR)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(godot-module-visual_shader PRIVATE')
    #   for file in list_files(cog, 'editor'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(godot-module-visual_shader PRIVATE
            editor/visual_shader_editor_plugin.cpp
            editor/visual_shader_editor_plugin.h
            editor/visual_shader_language_plugin.cpp
            editor/visual_shader_language_plugin.h
    )
    # [[[end]]]
endif ()

target_link_libraries(godot-module-visual_shader PUBLIC core)
group_target_sources(godot-module-visual_shader)
