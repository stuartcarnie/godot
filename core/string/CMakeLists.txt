godot_add_library(godot-string FOLDER core STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-string PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-string PRIVATE
        char_utils.h
        locales.h
        node_path.cpp
        node_path.h
        optimized_translation.cpp
        optimized_translation.h
        print_string.cpp
        print_string.h
        string_buffer.h
        string_builder.cpp
        string_builder.h
        string_name.cpp
        string_name.h
        translation.cpp
        translation.h
        translation_po.cpp
        translation_po.h
        ucaps.h
        ustring.cpp
        ustring.h
)
# [[[end]]]
target_include_directories(godot-string PUBLIC ${GODOT_ENGINE_ROOT_DIRECTORY})
target_link_libraries(godot-string PUBLIC godot-object godot-thirdparty-misc)
