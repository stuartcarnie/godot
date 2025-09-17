godot_add_library(harfbuzz FOLDER thirdparty STATIC)

target_sources(harfbuzz PRIVATE
        src/hb-aat-layout.cc
        src/hb-aat-map.cc
        src/hb-blob.cc
        src/hb-buffer-serialize.cc
        src/hb-buffer-verify.cc
        src/hb-buffer.cc
        # src/hb-cairo-utils.cc
        # src/hb-cairo.cc
        src/hb-common.cc
        # src/hb-coretext-font.cc
        # src/hb-coretext-shape.cc
        # src/hb-directwrite.cc
        src/hb-draw.cc
        src/hb-face-builder.cc
        src/hb-face.cc
        src/hb-fallback-shape.cc
        src/hb-font.cc
        # src/hb-gdi.cc
        # src/hb-glib.cc
        # src/hb-gobject-structs.cc
        src/hb-icu.cc
        src/hb-map.cc
        src/hb-number.cc
        src/hb-ot-cff1-table.cc
        src/hb-ot-cff2-table.cc
        src/hb-ot-color.cc
        src/hb-ot-face.cc
        src/hb-ot-font.cc
        src/hb-ot-layout.cc
        src/hb-ot-map.cc
        src/hb-ot-math.cc
        src/hb-ot-meta.cc
        src/hb-ot-metrics.cc
        src/hb-ot-name.cc
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
        src/hb-ot-shape-fallback.cc
        src/hb-ot-shape-normalize.cc
        src/hb-ot-shape.cc
        src/hb-ot-tag.cc
        src/hb-ot-var.cc
        src/hb-outline.cc
        src/hb-paint-bounded.cc
        src/hb-paint-extents.cc
        src/hb-paint.cc
        src/hb-set.cc
        src/hb-shape-plan.cc
        src/hb-shape.cc
        src/hb-shaper.cc
        src/hb-static.cc
        src/hb-style.cc
        src/hb-subset-cff-common.cc
        src/hb-subset-cff1.cc
        src/hb-subset-cff2.cc
        src/hb-subset-input.cc
        src/hb-subset-instancer-iup.cc
        src/hb-subset-instancer-solver.cc
        src/hb-subset-plan.cc
        src/hb-subset-serialize.cc
        src/hb-subset.cc
        src/hb-ucd.cc
        src/hb-unicode.cc
        # src/hb-uniscribe.cc
        src/OT/Var/VARC/VARC.cc

        # TODO: should be conditional if free type is enabled
        src/hb-ft.cc

        # TODO: if graphite is enabled
        src/hb-graphite2.cc
)

target_include_directories(harfbuzz PUBLIC src)
target_compile_definitions(harfbuzz PRIVATE
        # TODO: should be conditional if free type is enabled
        HAVE_FREETYPE
        # TODO: if graphite is enabled
        HAVE_GRAPHITE2
        HAVE_ICU)
target_compile_options(harfbuzz PRIVATE $<$<COMPILE_LANGUAGE:CXX,C,OBJC,OBJCXX>:$<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>>)
target_link_libraries(harfbuzz PUBLIC freetype graphite icu4c)
