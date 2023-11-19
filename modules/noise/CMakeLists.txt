godot_add_library(godot-module-noise FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-noise PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-noise PRIVATE
        fastnoise_lite.cpp
        fastnoise_lite.h
        noise.cpp
        noise.h
        noise_texture_2d.cpp
        noise_texture_2d.h
        noise_texture_3d.cpp
        noise_texture_3d.h
        register_types.cpp
        register_types.h
)
# [[[end]]]

if (WANT_EDITOR)
    # [[[cog
    #   import cog
    #   from cmake_builders import list_files
    #   cog.outl('target_sources(godot-module-noise PRIVATE')
    #   for file in list_files(cog, 'editor'):
    #       cog.outl(f'        {file}')
    #   cog.outl(')')
    # ]]]
    target_sources(godot-module-noise PRIVATE
            editor/noise_editor_plugin.cpp
            editor/noise_editor_plugin.h
    )
    # [[[end]]]
endif ()
target_link_libraries(godot-module-noise PUBLIC godot-object)
