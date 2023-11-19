godot_add_library(godot-module-upnp FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-upnp PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-upnp PRIVATE
        register_types.cpp
        register_types.h
        upnp.cpp
        upnp.h
        upnp_device.cpp
        upnp_device.h
)
# [[[end]]]
target_link_libraries(godot-module-upnp PUBLIC godot-object miniupnpc)
