godot_add_library(metal-cpp FOLDER thirdparty NO_INCLUDES STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(metal-cpp PRIVATE')
#   for file in list_files(cog, ['.', 'shaders']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(metal-cpp PRIVATE
        metal_cpp.cpp
)
# [[[end]]]
target_include_directories(metal-cpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(metal-cpp PUBLIC FFX_GCC)
