# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('set_abs(SHADERS ${CMAKE_CURRENT_SOURCE_DIR}')
#   for file in list_files(cog, '.', exts=['glsl']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
set_abs(SHADERS ${CMAKE_CURRENT_SOURCE_DIR}
        final_blit.glsl
)
# [[[end]]]

set(GENERATED_OUTPUT_DIR ${CMAKE_BINARY_INCLUDE_DIR}/modules/retrofx)
string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR} ${GENERATED_OUTPUT_DIR} SHADERS_GENERATED "${SHADERS}")
string(REPLACE "glsl" "glsl.gen.h" SHADERS_GENERATED "${SHADERS_GENERATED}")
generate_shader_sources(glsl "${SHADERS}" "${SHADERS_GENERATED}")

add_custom_target(generate_retrofx_rd_shaders DEPENDS ${SHADERS_GENERATED})
set_target_properties(generate_retrofx_rd_shaders PROPERTIES FOLDER "generated")

godot_add_library(godot-module-retrofx FOLDER modules STATIC)
add_dependencies(godot-module-retrofx generate_retrofx_rd_shaders)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-retrofx PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-retrofx PRIVATE
        constants.h
        filter_chain.cpp
        filter_chain.h
        register_types.cpp
        register_types.h
        retro_fx_rect.cpp
        retro_fx_rect.h
        rfx_shader_graph.cpp
        rfx_shader_graph.h
        shader_chain.cpp
        shader_chain.h
        shader_container.cpp
        shader_container.h
        shader_pass_bindings.cpp
        shader_pass_bindings.h
        shader_pass_semantics.cpp
        shader_pass_semantics.h
        slang_shader.cpp
        slang_shader.h
)
# [[[end]]]

if (WANT_EDITOR)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(godot-module-retrofx PRIVATE')
    #   for file in list_files(cog, 'editor'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(godot-module-retrofx PRIVATE
            editor/resource_importer_slang.cpp
            editor/resource_importer_slang.h
            editor/resource_importer_slang_preset.cpp
            editor/resource_importer_slang_preset.h
            editor/retro_fx_editor_plugin.cpp
            editor/retro_fx_editor_plugin.h
            editor/retro_fx_inspector_plugin.cpp
            editor/retro_fx_inspector_plugin.h
    )
    # [[[end]]]
endif ()

target_link_libraries(godot-module-retrofx PUBLIC core servers slang scene)
# disable -Wunused-variable for now, as it's too noisy
target_compile_options(godot-module-retrofx PRIVATE -Wno-unused-variable -Wno-unused-private-field -Wno-c99-designator -fexceptions)
group_target_sources(godot-module-retrofx)
