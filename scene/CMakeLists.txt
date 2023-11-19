add_subdirectory(2d)
add_subdirectory(3d)
add_subdirectory(animation)
add_subdirectory(audio)
add_subdirectory(debugger)
add_subdirectory(gui)
add_subdirectory(main)
add_subdirectory(resources)
add_subdirectory(theme)

godot_add_library(godot-scene FOLDER scene STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-scene PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-scene PRIVATE
        property_utils.cpp
        property_utils.h
        register_scene_types.cpp
        register_scene_types.h
        scene_string_names.cpp
        scene_string_names.h
)
# [[[end]]]

target_link_libraries(godot-scene PUBLIC godot-string godot-config godot-scene-resources godot-scene-audio)
