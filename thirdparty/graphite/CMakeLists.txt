godot_add_library(graphite FOLDER thirdparty STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(graphite PRIVATE')
#   for file in list_files(cog, ['src', 'include/graphite2']):
#       if file.name in ['direct_machine.cpp', 'call_machine.cpp']: continue
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
        src/FileFace.cpp
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
if((${CMAKE_SYSTEM_NAME} STREQUAL "Windows") AND (${MSVC}))
    target_sources(graphite PRIVATE src/call_machine.cpp)
else()
    target_sources(graphite PRIVATE src/direct_machine.cpp)
endif()
target_include_directories(graphite PUBLIC include)
target_include_directories(graphite PRIVATE src)

