godot_add_library(godot-module-webp FOLDER modules STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-module-webp PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-module-webp PRIVATE
        image_loader_webp.cpp
        image_loader_webp.h
        register_types.cpp
        register_types.h
        resource_saver_webp.cpp
        resource_saver_webp.h
        webp_common.cpp
        webp_common.h
)
# [[[end]]]
target_link_libraries(godot-module-webp PUBLIC godot-object webp)
