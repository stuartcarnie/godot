godot_add_library(godot-scene-main FOLDER scene STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-scene-main PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-scene-main PRIVATE
        canvas_item.cpp
        canvas_item.h
        canvas_layer.cpp
        canvas_layer.h
        http_request.cpp
        http_request.h
        instance_placeholder.cpp
        instance_placeholder.h
        missing_node.cpp
        missing_node.h
        multiplayer_api.cpp
        multiplayer_api.h
        multiplayer_peer.cpp
        multiplayer_peer.h
        node.cpp
        node.h
        resource_preloader.cpp
        resource_preloader.h
        scene_tree.cpp
        scene_tree.h
        shader_globals_override.cpp
        shader_globals_override.h
        timer.cpp
        timer.h
        viewport.cpp
        viewport.h
        window.cpp
        window.h
)
# [[[end]]]
target_link_libraries(godot-scene-main PUBLIC godot-io godot-string godot-object godot-os godot-scene-debugger godot-scene-animation godot-scene-gui)
