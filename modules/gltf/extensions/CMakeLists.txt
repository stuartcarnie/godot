# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-gltf PRIVATE')
#   for file in list_files(cog, ['.', 'physics']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-gltf PRIVATE
        gltf_document_extension.cpp
        gltf_document_extension.h
        gltf_document_extension_convert_importer_mesh.cpp
        gltf_document_extension_convert_importer_mesh.h
        gltf_document_extension_texture_ktx.cpp
        gltf_document_extension_texture_ktx.h
        gltf_document_extension_texture_webp.cpp
        gltf_document_extension_texture_webp.h
        gltf_light.cpp
        gltf_light.h
        gltf_spec_gloss.cpp
        gltf_spec_gloss.h
        physics/gltf_document_extension_physics.cpp
        physics/gltf_document_extension_physics.h
        physics/gltf_physics_body.cpp
        physics/gltf_physics_body.h
        physics/gltf_physics_shape.cpp
        physics/gltf_physics_shape.h
)
# [[[end]]]

