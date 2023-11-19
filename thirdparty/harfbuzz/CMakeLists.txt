godot_add_library(harfbuzz FOLDER thirdparty STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(harfbuzz PRIVATE')
#   for file in list_files(cog, ['src', 'src/graph']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(harfbuzz PRIVATE
        src/graph/gsubgpos-context.cc
        src/hb-aat-layout.cc
        src/hb-aat-layout.h
        src/hb-aat-map.cc
        src/hb-aat.h
        src/hb-blob.cc
        src/hb-blob.h
        src/hb-buffer-serialize.cc
        src/hb-buffer-verify.cc
        src/hb-buffer.cc
        src/hb-buffer.h
        src/hb-cairo-utils.cc
        src/hb-cairo.cc
        src/hb-cairo.h
        src/hb-common.cc
        src/hb-common.h
        src/hb-coretext.cc
        src/hb-coretext.h
        src/hb-deprecated.h
        src/hb-directwrite.cc
        src/hb-directwrite.h
        src/hb-draw.cc
        src/hb-draw.h
        src/hb-face-builder.cc
        src/hb-face.cc
        src/hb-face.h
        src/hb-fallback-shape.cc
        src/hb-font.cc
        src/hb-font.h
        src/hb-ft.cc
        src/hb-ft.h
        src/hb-gdi.cc
        src/hb-gdi.h
        src/hb-glib.cc
        src/hb-glib.h
        src/hb-gobject-structs.cc
        src/hb-gobject-structs.h
        src/hb-gobject.h
        src/hb-graphite2.cc
        src/hb-graphite2.h
        src/hb-icu.cc
        src/hb-icu.h
        src/hb-map.cc
        src/hb-map.h
        src/hb-number.cc
        src/hb-ot-cff1-table.cc
        src/hb-ot-cff2-table.cc
        src/hb-ot-color.cc
        src/hb-ot-color.h
        src/hb-ot-deprecated.h
        src/hb-ot-face.cc
        src/hb-ot-font.cc
        src/hb-ot-font.h
        src/hb-ot-layout.cc
        src/hb-ot-layout.h
        src/hb-ot-map.cc
        src/hb-ot-math.cc
        src/hb-ot-math.h
        src/hb-ot-meta.cc
        src/hb-ot-meta.h
        src/hb-ot-metrics.cc
        src/hb-ot-metrics.h
        src/hb-ot-name.cc
        src/hb-ot-name.h
        src/hb-ot-shape-fallback.cc
        src/hb-ot-shape-normalize.cc
        src/hb-ot-shape.cc
        src/hb-ot-shape.h
        src/hb-ot-shaper-arabic.cc
        src/hb-ot-shaper-default.cc
        src/hb-ot-shaper-hangul.cc
        src/hb-ot-shaper-hebrew.cc
        src/hb-ot-shaper-indic-table.cc
        src/hb-ot-shaper-indic.cc
        src/hb-ot-shaper-khmer.cc
        src/hb-ot-shaper-myanmar.cc
        src/hb-ot-shaper-syllabic.cc
        src/hb-ot-shaper-thai.cc
        src/hb-ot-shaper-use.cc
        src/hb-ot-shaper-vowel-constraints.cc
        src/hb-ot-tag.cc
        src/hb-ot-var.cc
        src/hb-ot-var.h
        src/hb-ot.h
        src/hb-outline.cc
        src/hb-paint-extents.cc
        src/hb-paint.cc
        src/hb-paint.h
        src/hb-set.cc
        src/hb-set.h
        src/hb-shape-plan.cc
        src/hb-shape-plan.h
        src/hb-shape.cc
        src/hb-shape.h
        src/hb-shaper.cc
        src/hb-static.cc
        src/hb-style.cc
        src/hb-style.h
        src/hb-subset-cff-common.cc
        src/hb-subset-cff1.cc
        src/hb-subset-cff2.cc
        src/hb-subset-input.cc
        src/hb-subset-instancer-solver.cc
        src/hb-subset-plan.cc
        src/hb-subset-repacker.cc
        src/hb-subset-repacker.h
        src/hb-subset.cc
        src/hb-subset.h
        src/hb-ucd.cc
        src/hb-unicode.cc
        src/hb-unicode.h
        src/hb-uniscribe.cc
        src/hb-uniscribe.h
        src/hb-version.h
        src/hb.h
)
# [[[end]]]
target_include_directories(harfbuzz PUBLIC src)
target_compile_definitions(harfbuzz PRIVATE HAVE_FREETYPE=1 HAVE_ICU=1)
target_compile_options(harfbuzz PRIVATE $<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>)
target_link_libraries(harfbuzz PUBLIC freetype icu4c)
