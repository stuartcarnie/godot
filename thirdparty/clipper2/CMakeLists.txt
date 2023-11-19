godot_add_library(clipper2 FOLDER thirdparty STATIC)

# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(clipper2 PRIVATE')
#   for file in list_files(cog, 'src'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(clipper2 PRIVATE
        src/clipper.engine.cpp
        src/clipper.offset.cpp
        src/clipper.rectclip.cpp
)
# [[[end]]]

target_include_directories(clipper2 PUBLIC include)
target_compile_definitions(clipper2 PUBLIC CLIPPER2_ENABLED)
