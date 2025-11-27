# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(core PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(core PRIVATE
        extension_api_dump.cpp
        extension_api_dump.h
        gdextension.cpp
        gdextension.h
        gdextension_function_loader.cpp
        gdextension_function_loader.h
        gdextension_interface.cpp
        gdextension_interface_header_generator.cpp
        gdextension_interface_header_generator.h
        gdextension_library_loader.cpp
        gdextension_library_loader.h
        gdextension_loader.h
        gdextension_manager.cpp
        gdextension_manager.h
        gdextension_special_compat_hashes.cpp
        gdextension_special_compat_hashes.h
        godot_instance.cpp
        godot_instance.h
        libgodot.h
)
# [[[end]]]

target_include_directories(core PUBLIC
        ${CMAKE_BINARY_INCLUDE_DIR}/core/extension)
