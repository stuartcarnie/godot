add_subdirectory(direct_states)
add_subdirectory(queries)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(servers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(servers PRIVATE
        physics_server_3d.cpp
        physics_server_3d.h
        physics_server_3d_constants.h
        physics_server_3d_dummy.h
        physics_server_3d_enums.h
        physics_server_3d_extension.cpp
        physics_server_3d_extension.h
        physics_server_3d_manager.cpp
        physics_server_3d_manager.h
        physics_server_3d_rendering_server_handler.cpp
        physics_server_3d_rendering_server_handler.h
        physics_server_3d_types.h
        physics_server_3d_wrap_mt.cpp
        physics_server_3d_wrap_mt.h
)
# [[[end]]]
