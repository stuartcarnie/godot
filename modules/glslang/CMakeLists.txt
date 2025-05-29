godot_add_library(godot-module-glslang FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-glslang PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-glslang PRIVATE
        register_types.cpp
        register_types.h
        shader_compile.h
)
# [[[end]]]
target_link_libraries(godot-module-glslang PUBLIC core godot-glslang)
group_target_sources(godot-module-glslang)
