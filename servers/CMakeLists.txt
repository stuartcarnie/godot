cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

add_subdirectory(audio)
add_subdirectory(camera)
add_subdirectory(debugger)
add_subdirectory(display)
add_subdirectory(extensions)
add_subdirectory(movie_writer)
add_subdirectory(navigation)
add_subdirectory(physics_2d)
add_subdirectory(physics_3d)
add_subdirectory(rendering)
add_subdirectory(text)
add_subdirectory(xr)

godot_add_library(godot-servers FOLDER servers STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-servers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-servers PRIVATE
        audio_server.cpp
        audio_server.h
        camera_server.cpp
        camera_server.h
        display_server.cpp
        display_server.h
        display_server_headless.h
        navigation_server_2d.cpp
        navigation_server_2d.h
        navigation_server_2d_dummy.h
        navigation_server_3d.cpp
        navigation_server_3d.h
        navigation_server_3d_dummy.h
        physics_server_2d.cpp
        physics_server_2d.h
        physics_server_2d_wrap_mt.cpp
        physics_server_2d_wrap_mt.h
        physics_server_3d.cpp
        physics_server_3d.h
        physics_server_3d_wrap_mt.cpp
        physics_server_3d_wrap_mt.h
        register_server_types.cpp
        register_server_types.h
        rendering_server.cpp
        rendering_server.h
        server_wrap_mt_common.h
        text_server.cpp
        text_server.h
        xr_server.cpp
        xr_server.h
)
# [[[end]]]
target_link_libraries(godot-servers PUBLIC
        godot-object
        godot-input
        godot-io
        godot-config
        godot-math
        godot-server-audio
        godot-server-camera
        godot-server-renderer
        godot-server-navigation
        godot-server-physics-2D
        godot-server-physics-3D
        godot-server-movie-writer
        godot-server-debugger
        godot-server-xr
        godot-server-extensions
        godot-server-text)

