godot_add_library(godot-module-freetype FOLDER modules NO_INCLUDES INTERFACE)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-freetype PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-freetype PRIVATE
        register_types.h
)
# [[[end]]]
target_link_libraries(godot-module-freetype INTERFACE core freetype)
group_target_sources(godot-module-freetype)
