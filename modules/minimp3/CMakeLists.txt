godot_add_library(godot-module-minimp3 FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-minimp3 PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-minimp3 PRIVATE
        audio_stream_mp3.cpp
        audio_stream_mp3.h
        register_types.cpp
        register_types.h
        resource_importer_mp3.cpp
        resource_importer_mp3.h
)
# [[[end]]]
target_link_libraries(godot-module-minimp3 PUBLIC godot-object minimp3)
