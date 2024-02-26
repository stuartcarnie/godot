godot_add_library(godot-module-fbx FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-fbx PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-fbx PRIVATE
        fbx_document.cpp
        fbx_document.h
        fbx_state.cpp
        fbx_state.h
        register_types.cpp
        register_types.h
)
# [[[end]]]

if (WANT_EDITOR)
	# [[[cog
	#   import cog
	#   from cmake_builders import list_files
	#   cog.outl('target_sources(godot-module-fbx PRIVATE')
	#   for file in list_files(cog, 'editor'):
	#       cog.outl(f'        {file}')
	#   cog.outl(')')
	# ]]]
	target_sources(godot-module-fbx PRIVATE
	        editor/editor_scene_importer_fbx2gltf.cpp
	        editor/editor_scene_importer_fbx2gltf.h
	        editor/editor_scene_importer_ufbx.cpp
	        editor/editor_scene_importer_ufbx.h
	)
	# [[[end]]]
endif ()

target_link_libraries(godot-module-fbx PUBLIC godot-object ufbx)
