godot_add_library(godot-module-webxr FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-webxr PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-webxr PRIVATE
        godot_webxr.h
        register_types.cpp
        register_types.h
        webxr_interface.cpp
        webxr_interface.h
        webxr_interface_js.cpp
        webxr_interface_js.h
)
# [[[end]]]
target_link_libraries(godot-module-webxr PUBLIC godot-object)
