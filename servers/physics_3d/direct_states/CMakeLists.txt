# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(servers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(servers PRIVATE
        physics_direct_body_state_3d.cpp
        physics_direct_body_state_3d.h
        physics_direct_space_state_3d.cpp
        physics_direct_space_state_3d.h
)
# [[[end]]]
