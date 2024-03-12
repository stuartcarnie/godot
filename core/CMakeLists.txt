add_subdirectory(object)
add_subdirectory(math)
add_subdirectory(string)
add_subdirectory(config)
add_subdirectory(crypto)
add_subdirectory(debugger)
add_subdirectory(error)
add_subdirectory(extension)
add_subdirectory(input)
add_subdirectory(templates)
add_subdirectory(io)
add_subdirectory(os)
add_subdirectory(variant)

godot_add_library(godot-core FOLDER core STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-core PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-core PRIVATE
        core_bind.cpp
        core_bind.h
        core_constants.cpp
        core_constants.h
        core_globals.cpp
        core_globals.h
        core_string_names.cpp
        core_string_names.h
        doc_data.cpp
        doc_data.h
        register_core_types.cpp
        register_core_types.h
        typedefs.h
        version.h
)
# [[[end]]]

target_link_libraries(godot-core PUBLIC godot-object)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(godot-core PUBLIC platform-linuxbsd)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(godot-core PUBLIC platform-windows)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(godot-core PUBLIC platform-macos)
elseif (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    target_link_libraries(godot-core PUBLIC platform-ios)
else ()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif ()
