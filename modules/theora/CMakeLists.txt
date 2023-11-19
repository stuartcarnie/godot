godot_add_library(godot-module-theora FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-theora PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-theora PRIVATE
        register_types.cpp
        register_types.h
        video_stream_theora.cpp
        video_stream_theora.h
)
# [[[end]]]
target_link_libraries(godot-module-theora PUBLIC godot-object theora vorbis)
