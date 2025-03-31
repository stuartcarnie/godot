# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(servers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(servers PRIVATE
        render_forward_clustered.cpp
        render_forward_clustered.h
        scene_shader_forward_clustered.cpp
        scene_shader_forward_clustered.h
)
# [[[end]]]
