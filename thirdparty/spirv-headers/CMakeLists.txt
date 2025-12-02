godot_add_library(spirv-headers FOLDER thirdparty NO_INCLUDES INTERFACE)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(spirv-headers PRIVATE')
#   for file in list_files(cog, 'include/spirv/unified1/'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(spirv-headers PRIVATE
        include/spirv/unified1/spirv.h
        include/spirv/unified1/spirv.hpp
)
# [[[end]]]

target_include_directories(spirv-headers
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/spirv/unified1)
