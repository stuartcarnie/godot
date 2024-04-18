set(RESOURCES_DEFAULT_FONT_SOURCE ${GODOT_SOURCE_ROOT_THIRDPARTY_DIR}/fonts/OpenSans_SemiBold.woff2)
set(RESOURCES_DEFAULT_FONT_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/scene/resources/default_theme/default_font.gen.h)
generate_file(resource_scene_make_fonts_header ${RESOURCES_DEFAULT_FONT_SOURCE} ${RESOURCES_DEFAULT_FONT_OUTPUT})

add_subdirectory(2d)
add_subdirectory(3d)

godot_add_library(godot-scene-resources FOLDER scene STATIC ${RESOURCES_DEFAULT_FONT_OUTPUT})
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-scene-resources PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-scene-resources PRIVATE
        animated_texture.cpp
        animated_texture.h
        animation.cpp
        animation.h
        animation_library.cpp
        animation_library.h
        atlas_texture.cpp
        atlas_texture.h
        audio_stream_polyphonic.cpp
        audio_stream_polyphonic.h
        audio_stream_wav.cpp
        audio_stream_wav.h
        bit_map.cpp
        bit_map.h
        bone_map.cpp
        bone_map.h
        camera_attributes.cpp
        camera_attributes.h
        camera_texture.cpp
        camera_texture.h
        canvas_item_material.cpp
        canvas_item_material.h
        compositor.cpp
        compositor.h
        compressed_texture.cpp
        compressed_texture.h
        curve.cpp
        curve.h
        curve_texture.cpp
        curve_texture.h
        environment.cpp
        environment.h
        font.cpp
        font.h
        gradient.cpp
        gradient.h
        gradient_texture.cpp
        gradient_texture.h
        image_texture.cpp
        image_texture.h
        immediate_mesh.cpp
        immediate_mesh.h
        label_settings.cpp
        label_settings.h
        material.cpp
        material.h
        mesh.cpp
        mesh.h
        mesh_data_tool.cpp
        mesh_data_tool.h
        mesh_texture.cpp
        mesh_texture.h
        multimesh.cpp
        multimesh.h
        navigation_mesh.cpp
        navigation_mesh.h
        packed_scene.cpp
        packed_scene.h
        particle_process_material.cpp
        particle_process_material.h
        physics_material.cpp
        physics_material.h
        placeholder_textures.cpp
        placeholder_textures.h
        portable_compressed_texture.cpp
        portable_compressed_texture.h
        resource_format_text.cpp
        resource_format_text.h
        shader.cpp
        shader.h
        shader_include.cpp
        shader_include.h
        skeleton_profile.cpp
        skeleton_profile.h
        sky.cpp
        sky.h
        sprite_frames.cpp
        sprite_frames.h
        style_box.cpp
        style_box.h
        style_box_flat.cpp
        style_box_flat.h
        style_box_line.cpp
        style_box_line.h
        style_box_texture.cpp
        style_box_texture.h
        surface_tool.cpp
        surface_tool.h
        syntax_highlighter.cpp
        syntax_highlighter.h
        text_file.cpp
        text_file.h
        text_line.cpp
        text_line.h
        text_paragraph.cpp
        text_paragraph.h
        texture.cpp
        texture.h
        texture_rd.cpp
        texture_rd.h
        theme.cpp
        theme.h
        video_stream.cpp
        video_stream.h
        visual_shader.cpp
        visual_shader.h
        visual_shader_nodes.cpp
        visual_shader_nodes.h
        visual_shader_particle_nodes.cpp
        visual_shader_particle_nodes.h
        visual_shader_sdf_nodes.cpp
        visual_shader_sdf_nodes.h
        world_2d.cpp
        world_2d.h
)
# [[[end]]]
target_link_libraries(godot-scene-resources PUBLIC godot-templates godot-io godot-variant godot-scene-main)
target_include_directories(godot-scene-resources PUBLIC ${CMAKE_BINARY_INCLUDE_DIR}/scene/resources/default_theme)
