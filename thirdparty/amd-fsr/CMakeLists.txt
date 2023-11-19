godot_add_library(amd-fsr FOLDER NO_INCLUDES thirdparty INTERFACE)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(amd-fsr PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(amd-fsr PRIVATE
        ffx_a.h
        ffx_fsr1.h
)
# [[[end]]]

target_include_directories(amd-fsr INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
