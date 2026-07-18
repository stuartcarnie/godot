# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(servers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(servers PRIVATE
        navigation_constants_2d.h
        navigation_path_query_parameters_2d.cpp
        navigation_path_query_parameters_2d.h
        navigation_path_query_result_2d.cpp
        navigation_path_query_result_2d.h
        navigation_server_2d.cpp
        navigation_server_2d.h
        navigation_server_2d_dummy.h
        navigation_server_2d_manager.cpp
        navigation_server_2d_manager.h
)
# [[[end]]]
