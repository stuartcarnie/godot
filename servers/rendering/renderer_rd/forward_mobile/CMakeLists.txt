# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(servers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(servers PRIVATE
        render_forward_mobile.cpp
        render_forward_mobile.h
        scene_shader_forward_mobile.cpp
        scene_shader_forward_mobile.h
)
# [[[end]]]
