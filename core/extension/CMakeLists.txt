godot_add_library(godot-extension FOLDER core STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-extension PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-extension PRIVATE
        extension_api_dump.cpp
        extension_api_dump.h
        gdextension.cpp
        gdextension.h
        gdextension_compat_hashes.cpp
        gdextension_compat_hashes.h
        gdextension_interface.cpp
        gdextension_interface.h
        gdextension_manager.cpp
        gdextension_manager.h
)
# [[[end]]]
target_link_libraries(godot-extension PUBLIC godot-object)