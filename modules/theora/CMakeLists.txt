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

if (WANT_EDITOR)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(godot-module-theora PRIVATE')
    #   for file in list_files(cog, 'editor'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(godot-module-theora PRIVATE
            editor/movie_writer_ogv.cpp
            editor/movie_writer_ogv.h
            editor/rgb2yuv.h
    )
    # [[[end]]]
endif ()

target_link_libraries(godot-module-theora PUBLIC core theora vorbis)
group_target_sources(godot-module-theora)
