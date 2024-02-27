godot_add_library(godot-scene-2d-physics-joints FOLDER scene STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-scene-2d-physics-joints PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-scene-2d-physics-joints PRIVATE
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
target_link_libraries(godot-scene-2d-physics-joints PRIVATE godot-config)

