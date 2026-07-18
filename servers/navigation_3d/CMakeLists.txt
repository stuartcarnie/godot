# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(servers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(servers PRIVATE
        navigation_constants_3d.h
        navigation_path_query_parameters_3d.cpp
        navigation_path_query_parameters_3d.h
        navigation_path_query_result_3d.cpp
        navigation_path_query_result_3d.h
        navigation_server_3d.cpp
        navigation_server_3d.h
        navigation_server_3d_dummy.h
        navigation_server_3d_manager.cpp
        navigation_server_3d_manager.h
)
# [[[end]]]
