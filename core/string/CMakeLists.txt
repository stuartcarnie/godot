# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(core PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(core PRIVATE
        alt_codes.h
        char_utils.h
        fuzzy_search.cpp
        fuzzy_search.h
        locales.h
        node_path.cpp
        node_path.h
        optimized_translation.cpp
        optimized_translation.h
        plural_rules.cpp
        plural_rules.h
        print_string.cpp
        print_string.h
        string_buffer.h
        string_builder.cpp
        string_builder.h
        string_name.cpp
        string_name.h
        translation.cpp
        translation.h
        translation_domain.cpp
        translation_domain.h
        translation_po.cpp
        translation_po.h
        translation_server.cpp
        translation_server.h
        ucaps.h
        ustring.cpp
        ustring.h
)
# [[[end]]]
target_link_libraries(core PRIVATE godot-thirdparty-misc)
