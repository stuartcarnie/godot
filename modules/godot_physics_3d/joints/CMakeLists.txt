# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-physics-3D PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-physics-3D PRIVATE
        godot_cone_twist_joint_3d.cpp
        godot_cone_twist_joint_3d.h
        godot_generic_6dof_joint_3d.cpp
        godot_generic_6dof_joint_3d.h
        godot_hinge_joint_3d.cpp
        godot_hinge_joint_3d.h
        godot_jacobian_entry_3d.h
        godot_pin_joint_3d.cpp
        godot_pin_joint_3d.h
        godot_slider_joint_3d.cpp
        godot_slider_joint_3d.h
)
# [[[end]]]
