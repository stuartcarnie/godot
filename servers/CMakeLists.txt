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
        $<$<BOOL:${ENABLE_VULKAN}>:vulkan>
        $<$<BOOL:${ENABLE_METAL}>:metal>
)

target_compile_definitions(servers
        PRIVATE
        $<$<BOOL:${ENABLE_RENDERING_DEVICE}>:RD_ENABLED> $<$<BOOL:${ENABLE_FORWARD_PLUS_RENDERER}>:FORWARD_RD_ENABLED> $<$<BOOL:${ENABLE_MOBILE_RENDERER}>:MOBILE_RD_ENABLED>
        $<$<BOOL:${ENABLE_DYNAMIC_CORE_SHADERS}>:DYNAMIC_CORE_SHADERS GODOT_SOURCE_ROOT="${CMAKE_SOURCE_DIR}">
        $<$<AND:$<BOOL:${ENABLE_VULKAN}>,$<BOOL:${VULKAN_FOUND}>>:VULKAN_ENABLED>
        $<$<BOOL:${ENABLE_METAL}>:METAL_ENABLED>
)

# This flag ensures that uninitialized stack variables are initialized to a known pattern.
# target_compile_options(servers PRIVATE -ftrivial-auto-var-init=pattern)

group_target_sources(servers)
