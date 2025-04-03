# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(scene PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(scene PRIVATE
        navigation_agent_3d.cpp
        navigation_agent_3d.h
        navigation_link_3d.cpp
        navigation_link_3d.h
        navigation_obstacle_3d.cpp
        navigation_obstacle_3d.h
        navigation_region_3d.cpp
        navigation_region_3d.h
)
# [[[end]]]

