# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(scene PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(scene PRIVATE
        cone_twist_joint_3d.cpp
        cone_twist_joint_3d.h
        generic_6dof_joint_3d.cpp
        generic_6dof_joint_3d.h
        hinge_joint_3d.cpp
        hinge_joint_3d.h
        joint_3d.cpp
        joint_3d.h
        pin_joint_3d.cpp
        pin_joint_3d.h
        slider_joint_3d.cpp
        slider_joint_3d.h
)
# [[[end]]]

