godot_add_library(servers STATIC)

add_subdirectory(audio)
add_subdirectory(camera)
add_subdirectory(debugger)
add_subdirectory(display)
add_subdirectory(movie_writer)
add_subdirectory(navigation_2d)
add_subdirectory(navigation_3d)
add_subdirectory(physics_2d)
add_subdirectory(physics_3d)
add_subdirectory(rendering)
add_subdirectory(text)
add_subdirectory(xr)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(servers PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(servers PRIVATE
        nav_heap.h
        register_server_types.cpp
        register_server_types.h
        server_wrap_mt_common.h
)
# [[[end]]]
target_link_libraries(servers PUBLIC
        core
        $<$<BOOL:${WANT_VULKAN}>:vulkan>
        $<$<BOOL:${WANT_METAL}>:metal>
)

target_compile_definitions(servers
        PRIVATE
        $<$<BOOL:${WANT_DYNAMIC_CORE_SHADERS}>:DYNAMIC_CORE_SHADERS GODOT_SOURCE_ROOT="${CMAKE_SOURCE_DIR}">
        $<$<AND:$<BOOL:${WANT_VULKAN}>,$<BOOL:${VULKAN_FOUND}>>:VULKAN_ENABLED=1 RD_ENABLED=1>
        $<$<BOOL:${WANT_METAL}>:METAL_ENABLED=1 RD_ENABLED=1>
)

# This flag ensures that uninitialized stack variables are initialized to a known pattern.
# target_compile_options(servers PRIVATE -ftrivial-auto-var-init=pattern)

group_target_sources(servers)
