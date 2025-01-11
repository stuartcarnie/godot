godot_add_library(graphite FOLDER thirdparty STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(graphite PRIVATE')
#   for file in list_files(cog, ['src', 'include/graphite2'], exclude=['src/FileFace.cpp', 'src/call_machine.cpp', 'src/direct_machine.cpp']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(graphite PRIVATE
        include/graphite2/Font.h
        include/graphite2/Log.h
        include/graphite2/Segment.h
        include/graphite2/Types.h
        src/CmapCache.cpp
        src/Code.cpp
        src/Collider.cpp
        src/Decompressor.cpp
        src/Face.cpp
        src/FeatureMap.cpp
        src/Font.cpp
        src/GlyphCache.cpp
        src/GlyphFace.cpp
        src/Intervals.cpp
        src/Justifier.cpp
        src/NameTable.cpp
        src/Pass.cpp
        src/Position.cpp
        src/Segment.cpp
        src/Silf.cpp
        src/Slot.cpp
        src/Sparse.cpp
        src/TtfUtil.cpp
        src/UtfCodec.cpp
        src/gr_char_info.cpp
        src/gr_face.cpp
        src/gr_features.cpp
        src/gr_font.cpp
        src/gr_logging.cpp
        src/gr_segment.cpp
        src/gr_slot.cpp
        src/json.cpp
)
# [[[end]]]
target_sources(graphite PRIVATE
        $<IF:$<AND:$<PLATFORM_ID:Windows>,$<CXX_COMPILER_ID:MSVC>>,
            # for windows with MSVC
            src/call_machine.cpp,
            # for everything else
            src/direct_machine.cpp>
)
target_include_directories(graphite
        PUBLIC include
        PRIVATE src
)
target_compile_definitions(graphite
        PUBLIC GRAPHITE2_STATIC GRAPHITE2_NTRACING GRAPHITE2_NFILEFACE
)
