# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(scene PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(scene PRIVATE
        damped_spring_joint_2d.cpp
        damped_spring_joint_2d.h
        groove_joint_2d.cpp
        groove_joint_2d.h
        joint_2d.cpp
        joint_2d.h
        pin_joint_2d.cpp
        pin_joint_2d.h
)
# [[[end]]]

