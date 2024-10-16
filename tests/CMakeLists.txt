cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

godot_add_library(tests FOLDER tests STATIC)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(tests PRIVATE')
#   for file in list_files(cog, '.', recursive=True):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(tests PRIVATE
        core/config/test_project_settings.h
        core/input/test_input_event.h
        core/input/test_input_event_key.h
        core/input/test_input_event_mouse.h
        core/input/test_shortcut.h
        core/io/test_config_file.h
        core/io/test_file_access.h
        core/io/test_http_client.h
        core/io/test_image.h
        core/io/test_ip.h
        core/io/test_json.h
        core/io/test_json_native.h
        core/io/test_marshalls.h
        core/io/test_packet_peer.h
        core/io/test_pck_packer.h
        core/io/test_resource.h
        core/io/test_stream_peer.h
        core/io/test_stream_peer_buffer.h
        core/io/test_xml_parser.h
        core/math/test_aabb.h
        core/math/test_astar.h
        core/math/test_basis.h
        core/math/test_color.h
        core/math/test_expression.h
        core/math/test_geometry_2d.h
        core/math/test_geometry_3d.h
        core/math/test_math_funcs.h
        core/math/test_plane.h
        core/math/test_quaternion.h
        core/math/test_random_number_generator.h
        core/math/test_rect2.h
        core/math/test_rect2i.h
        core/math/test_transform_2d.h
        core/math/test_transform_3d.h
        core/math/test_vector2.h
        core/math/test_vector2i.h
        core/math/test_vector3.h
        core/math/test_vector3i.h
        core/math/test_vector4.h
        core/math/test_vector4i.h
        core/object/test_class_db.h
        core/object/test_method_bind.h
        core/object/test_object.h
        core/object/test_undo_redo.h
        core/os/test_os.h
        core/string/test_node_path.h
        core/string/test_string.h
        core/string/test_translation.h
        core/string/test_translation_server.h
        core/templates/test_command_queue.h
        core/templates/test_hash_map.h
        core/templates/test_hash_set.h
        core/templates/test_list.h
        core/templates/test_local_vector.h
        core/templates/test_lru.h
        core/templates/test_oa_hash_map.h
        core/templates/test_paged_array.h
        core/templates/test_rid.h
        core/templates/test_vector.h
        core/test_crypto.h
        core/test_ctz.h
        core/test_hashing_context.h
        core/test_time.h
        core/threads/test_worker_thread_pool.h
        core/variant/test_array.h
        core/variant/test_callable.h
        core/variant/test_dictionary.h
        core/variant/test_variant.h
        core/variant/test_variant_utility.h
        display_server_mock.h
        scene/test_animation.h
        scene/test_arraymesh.h
        scene/test_audio_stream_wav.h
        scene/test_bit_map.h
        scene/test_button.h
        scene/test_camera_2d.h
        scene/test_camera_3d.h
        scene/test_code_edit.h
        scene/test_color_picker.h
        scene/test_control.h
        scene/test_curve.h
        scene/test_curve_2d.h
        scene/test_curve_3d.h
        scene/test_gradient.h
        scene/test_gradient_texture.h
        scene/test_graph_node.h
        scene/test_height_map_shape_3d.h
        scene/test_image_texture.h
        scene/test_image_texture_3d.h
        scene/test_instance_placeholder.h
        scene/test_navigation_agent_2d.h
        scene/test_navigation_agent_3d.h
        scene/test_navigation_obstacle_2d.h
        scene/test_navigation_obstacle_3d.h
        scene/test_navigation_region_2d.h
        scene/test_navigation_region_3d.h
        scene/test_node.h
        scene/test_node_2d.h
        scene/test_option_button.h
        scene/test_packed_scene.h
        scene/test_parallax_2d.h
        scene/test_path_2d.h
        scene/test_path_3d.h
        scene/test_path_follow_2d.h
        scene/test_path_follow_3d.h
        scene/test_physics_material.h
        scene/test_primitives.h
        scene/test_skeleton_3d.h
        scene/test_sky.h
        scene/test_sprite_frames.h
        scene/test_style_box_texture.h
        scene/test_tab_bar.h
        scene/test_tab_container.h
        scene/test_text_edit.h
        scene/test_theme.h
        scene/test_timer.h
        scene/test_tree.h
        scene/test_viewport.h
        scene/test_visual_shader.h
        scene/test_window.h
        servers/rendering/test_shader_preprocessor.h
        servers/test_navigation_server_2d.h
        servers/test_navigation_server_3d.h
        servers/test_text_server.h
        test_macros.cpp
        test_macros.h
        test_main.cpp
        test_main.h
        test_tools.h
        test_utils.cpp
        test_utils.h
        test_validate_testing.h
)
# [[[end]]]

target_link_libraries(tests PRIVATE godot-config doctest)
target_compile_definitions(tests
        PRIVATE
        $<$<BOOL:${DISABLE_EXCEPTIONS}>:DOCTEST_CONFIG_NO_EXCEPTIONS_BUT_WITH_ALL_ASSERTS>
        GODOT_SOURCE_ROOT="${CMAKE_SOURCE_DIR}"
)
