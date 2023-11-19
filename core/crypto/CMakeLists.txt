godot_add_library(godot-crypto FOLDER core STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-crypto PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-crypto PRIVATE
        aes_context.cpp
        aes_context.h
        crypto.cpp
        crypto.h
        crypto_core.cpp
        crypto_core.h
        hashing_context.cpp
        hashing_context.h
)
# [[[end]]]
target_include_directories(godot-crypto PUBLIC ${GODOT_ENGINE_ROOT_DIRECTORY})
target_link_libraries(godot-crypto PUBLIC godot-object mbedtls godot-io)
