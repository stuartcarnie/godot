# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(core PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(core PRIVATE
        aes_context.cpp
        aes_context.h
        crypto.cpp
        crypto.h
        crypto_core.cpp
        crypto_core.h
        crypto_resource_format.cpp
        crypto_resource_format.h
        hashing_context.cpp
        hashing_context.h
        ${CMAKE_SOURCE_DIR}/thirdparty/mbedtls/godot/godot_mbedtls_platform.cpp
)
# [[[end]]]
target_link_libraries(core PUBLIC mbedtls)
