godot_add_library(scene STATIC)

add_subdirectory(2d)
add_subdirectory(3d)
add_subdirectory(animation)
add_subdirectory(audio)
add_subdirectory(debugger)
add_subdirectory(gui)
add_subdirectory(main)
add_subdirectory(resources)
add_subdirectory(theme)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(scene PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(scene PRIVATE
        property_list_helper.cpp
        property_list_helper.h
        property_utils.cpp
        property_utils.h
        register_scene_types.cpp
        register_scene_types.h
        scene_string_names.h
)
# [[[end]]]

target_link_libraries(scene PUBLIC core)

group_target_sources(scene)
