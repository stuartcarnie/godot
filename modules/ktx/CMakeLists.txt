godot_add_library(godot-module-ktx FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-ktx PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-ktx PRIVATE
        register_types.cpp
        register_types.h
        texture_loader_ktx.cpp
        texture_loader_ktx.h
)
# [[[end]]]

target_link_libraries(godot-module-ktx PUBLIC ktx)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(godot-module-ktx PRIVATE platform-linuxbsd)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(godot-module-ktx PRIVATE platform-windows)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(godot-module-ktx PRIVATE platform-macos)
elseif (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    target_link_libraries(godot-module-ktx PRIVATE platform-ios)
else ()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif ()
