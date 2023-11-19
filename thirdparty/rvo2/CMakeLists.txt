godot_add_library(rvo2 FOLDER thirdparty STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(rvo2 PRIVATE')
#   for file in list_files(cog, ['rvo2_2d', 'rvo2_3d']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(rvo2 PRIVATE
        rvo2_2d/Agent2d.cpp
        rvo2_2d/Agent2d.h
        rvo2_2d/Definitions.h
        rvo2_2d/KdTree2d.cpp
        rvo2_2d/KdTree2d.h
        rvo2_2d/Obstacle2d.cpp
        rvo2_2d/Obstacle2d.h
        rvo2_2d/RVOSimulator2d.cpp
        rvo2_2d/RVOSimulator2d.h
        rvo2_2d/Vector2.h
        rvo2_3d/Agent3d.cpp
        rvo2_3d/Agent3d.h
        rvo2_3d/Definitions.h
        rvo2_3d/KdTree3d.cpp
        rvo2_3d/KdTree3d.h
        rvo2_3d/RVOSimulator3d.cpp
        rvo2_3d/RVOSimulator3d.h
        rvo2_3d/Vector3.h
)
# [[[end]]]
target_include_directories(rvo2 PUBLIC rvo2_2d rvo2_3d)
