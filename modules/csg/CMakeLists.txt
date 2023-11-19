godot_add_library(godot-module-csg FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-csg PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-csg PRIVATE
        csg.cpp
        csg.h
        csg_shape.cpp
        csg_shape.h
        register_types.cpp
        register_types.h
)
# [[[end]]]

if (WANT_EDITOR)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(godot-module-csg PRIVATE')
    #   for file in list_files(cog, 'editor'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(godot-module-csg PRIVATE
            editor/csg_gizmos.cpp
            editor/csg_gizmos.h
    )
    # [[[end]]]
endif ()

target_link_libraries(godot-module-csg PUBLIC godot-math godot-thirdparty-misc zlib)
