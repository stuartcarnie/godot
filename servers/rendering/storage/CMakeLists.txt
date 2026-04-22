set(RENDERING_STORAGE_LUTS_OUTPUT ${CMAKE_BINARY_INCLUDE_DIR}/servers/rendering/storage/ltc_lut.gen.h)
generate_lut_sources(
        ${CMAKE_CURRENT_SOURCE_DIR}/ltc/ltc_lut1.dds
        ${CMAKE_CURRENT_SOURCE_DIR}/ltc/ltc_lut2.dds
        ${RENDERING_STORAGE_LUTS_OUTPUT})

add_custom_target(servers-storage-generated
        DEPENDS
        ${RENDERING_STORAGE_LUTS_OUTPUT}
)
set_target_properties(servers-storage-generated PROPERTIES FOLDER "generated")
add_dependencies(servers servers-storage-generated)

target_sources(servers
        PRIVATE
        ${RENDERING_STORAGE_LUTS_OUTPUT}
)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(servers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(servers PRIVATE
        camera_attributes_storage.cpp
        camera_attributes_storage.h
        compositor_storage.cpp
        compositor_storage.h
        environment_storage.cpp
        environment_storage.h
        light_storage.h
        material_storage.h
        mesh_storage.cpp
        mesh_storage.h
        particles_storage.h
        render_data.cpp
        render_data.h
        render_data_extension.cpp
        render_data_extension.h
        render_scene_buffers.cpp
        render_scene_buffers.h
        render_scene_data.cpp
        render_scene_data.h
        texture_storage.h
        utilities.cpp
        utilities.h
        variant_converters.h
)
# [[[end]]]
