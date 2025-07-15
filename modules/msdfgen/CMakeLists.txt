godot_add_library(godot-module-msdfgen FOLDER modules NO_INCLUDES INTERFACE)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-msdfgen PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-msdfgen PRIVATE
        register_types.h
)
# [[[end]]]
target_link_libraries(godot-module-msdfgen INTERFACE core msdfgen)
group_target_sources(godot-module-msdfgen)
