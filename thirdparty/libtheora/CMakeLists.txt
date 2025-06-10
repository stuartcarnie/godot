godot_add_library(theora FOLDER thirdparty STATIC)
target_sources(theora PRIVATE
        bitpack.c
        decinfo.c
        decode.c
        dequant.c
        fragment.c
        huffdec.c
        idct.c
        info.c
        internal.c
        quant.c
        state.c
)

if (WANT_EDITOR)
    target_sources(theora PRIVATE
            analyze.c
            encfrag.c
            encinfo.c
            encode.c
            enquant.c
            fdct.c
            huffenc.c
            mathops.c
            mcenc.c
            rate.c
            tokenize.c
    )
endif ()

target_include_directories(theora PUBLIC .)
target_link_libraries(theora PUBLIC ogg)
set_property(TARGET theora PROPERTY C_STANDARD 17)
set_property(TARGET theora PROPERTY LANGUAGE CXX)
target_compile_options(theora PRIVATE $<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>) # disable all warnings
