godot_add_library(godot-module-etcpak FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-etcpak PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-etcpak PRIVATE
        image_compress_etcpak.cpp
        image_compress_etcpak.h
        register_types.cpp
        register_types.h
)
# [[[end]]]
target_link_libraries(godot-module-etcpak PUBLIC godot-io etcpak)
