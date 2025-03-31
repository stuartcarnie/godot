# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(scene PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(scene PRIVATE
        skeleton_modification_2d.cpp
        skeleton_modification_2d.h
        skeleton_modification_2d_ccdik.cpp
        skeleton_modification_2d_ccdik.h
        skeleton_modification_2d_fabrik.cpp
        skeleton_modification_2d_fabrik.h
        skeleton_modification_2d_jiggle.cpp
        skeleton_modification_2d_jiggle.h
        skeleton_modification_2d_lookat.cpp
        skeleton_modification_2d_lookat.h
        skeleton_modification_2d_physicalbones.cpp
        skeleton_modification_2d_physicalbones.h
        skeleton_modification_2d_stackholder.cpp
        skeleton_modification_2d_stackholder.h
        skeleton_modification_2d_twoboneik.cpp
        skeleton_modification_2d_twoboneik.h
        skeleton_modification_stack_2d.cpp
        skeleton_modification_stack_2d.h
)
# [[[end]]]

