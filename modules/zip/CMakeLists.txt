godot_add_library(godot-module-zip FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-zip PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-zip PRIVATE
        register_types.cpp
        register_types.h
        zip_packer.cpp
        zip_packer.h
        zip_reader.cpp
        zip_reader.h
)
# [[[end]]]
target_link_libraries(godot-module-zip PUBLIC godot-object godot-io minizip)
