add_subdirectory(extensions)

godot_add_library(godot-module-gltf FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-gltf PRIVATE')
#   for file in list_files(cog, ['.', 'structures']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-gltf PRIVATE
        gltf_defines.h
        gltf_document.cpp
        gltf_document.h
        gltf_state.cpp
        gltf_state.h
        gltf_template_convert.h
        register_types.cpp
        register_types.h
        skin_tool.cpp
        skin_tool.h
        structures/gltf_accessor.cpp
        structures/gltf_accessor.h
        structures/gltf_animation.cpp
        structures/gltf_animation.h
        structures/gltf_buffer_view.cpp
        structures/gltf_buffer_view.h
        structures/gltf_camera.cpp
        structures/gltf_camera.h
        structures/gltf_mesh.cpp
        structures/gltf_mesh.h
        structures/gltf_node.cpp
        structures/gltf_node.h
        structures/gltf_object_model_property.cpp
        structures/gltf_object_model_property.h
        structures/gltf_skeleton.cpp
        structures/gltf_skeleton.h
        structures/gltf_skin.cpp
        structures/gltf_skin.h
        structures/gltf_texture.cpp
        structures/gltf_texture.h
        structures/gltf_texture_sampler.cpp
        structures/gltf_texture_sampler.h
)
# [[[end]]]

if (WANT_EDITOR)
	# [[[cog
	#   import cog
	#   from cmake_builders import list_files
	#   cog.outl('target_sources(godot-module-gltf PRIVATE')
	#   for file in list_files(cog, 'editor'):
	#       cog.outl(f'        {file}')
	#   cog.outl(')')
	# ]]]
	target_sources(godot-module-gltf PRIVATE
	        editor/editor_import_blend_runner.cpp
	        editor/editor_import_blend_runner.h
	        editor/editor_scene_exporter_gltf_plugin.cpp
	        editor/editor_scene_exporter_gltf_plugin.h
	        editor/editor_scene_exporter_gltf_settings.cpp
	        editor/editor_scene_exporter_gltf_settings.h
	        editor/editor_scene_importer_blend.cpp
	        editor/editor_scene_importer_blend.h
	        editor/editor_scene_importer_gltf.cpp
	        editor/editor_scene_importer_gltf.h
	)
	# [[[end]]]
endif ()

target_link_libraries(godot-module-gltf PUBLIC godot-core minizip)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	target_link_libraries(godot-module-gltf PRIVATE shlwapi)
endif()
