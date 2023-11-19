godot_add_library(godot-module-mobile-vr FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-mobile-vr PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-mobile-vr PRIVATE
        mobile_vr_interface.cpp
        mobile_vr_interface.h
        register_types.cpp
        register_types.h
)
# [[[end]]]
target_link_libraries(godot-module-mobile-vr PUBLIC godot-object)
