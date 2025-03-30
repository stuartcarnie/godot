# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(godot-core PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(godot-core PRIVATE
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
target_link_libraries(godot-core PUBLIC mbedtls)
