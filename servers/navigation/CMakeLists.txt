godot_add_library(godot-server-navigation FOLDER servers STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-server-navigation PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-server-navigation PRIVATE
        navigation_path_query_parameters_2d.cpp
        navigation_path_query_parameters_2d.h
        navigation_path_query_parameters_3d.cpp
        navigation_path_query_parameters_3d.h
        navigation_path_query_result_2d.cpp
        navigation_path_query_result_2d.h
        navigation_path_query_result_3d.cpp
        navigation_path_query_result_3d.h
        navigation_utilities.h
)
# [[[end]]]
target_link_libraries(godot-server-navigation PUBLIC godot-object)
