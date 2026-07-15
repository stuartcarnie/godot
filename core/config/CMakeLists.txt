# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(core PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(core PRIVATE
        engine.cpp
        engine.h
        project_settings.cpp
        project_settings.h
)
# [[[end]]]

target_include_directories(core PUBLIC ${CMAKE_BINARY_INCLUDE_DIR})

target_compile_definitions(core
        PRIVATE
        $<$<AND:$<BOOL:${WANT_OPENGL}>,$<BOOL:OPENGL_FOUND}>>:GLES3_ENABLED>
        $<$<AND:$<BOOL:${WANT_VULKAN}>,$<BOOL:${VULKAN_FOUND}>>:VULKAN_ENABLED RD_ENABLED>
        $<$<BOOL:${WANT_METAL}>:METAL_ENABLED RD_ENABLED>
)
