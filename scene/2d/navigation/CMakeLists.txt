# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(scene PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(scene PRIVATE
        navigation_agent_2d.cpp
        navigation_agent_2d.h
        navigation_link_2d.cpp
        navigation_link_2d.h
        navigation_obstacle_2d.cpp
        navigation_obstacle_2d.h
        navigation_region_2d.cpp
        navigation_region_2d.h
)
# [[[end]]]

