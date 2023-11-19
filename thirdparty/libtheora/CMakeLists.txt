godot_add_library(theora FOLDER thirdparty STATIC)
target_sources(theora PRIVATE
        apiwrapper.h
        bitpack.c
        bitpack.h
        collect.h
        dct.h
        decinfo.c
        decint.h
        decode.c
        dequant.c
        dequant.h
        encint.h
        enquant.h
        fragment.c
        huffdec.c
        huffdec.h
        huffenc.h
        huffman.h
        idct.c
        info.c
        internal.c
        internal.h
        mathops.h
        modedec.h
        ocintrin.h
        quant.c
        quant.h
        state.c
        state.h
        theora/theoradec.h
        theora/codec.h
        theora/theora.h
        theora/theoraenc.h
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_sources(theora PRIVATE
            x86_vc/x86int.h
            x86_vc/x86state.c
            x86_vc/mmxstate.c
            x86_vc/x86enc.h
            x86_vc/x86cpu.h
            x86_vc/mmxfrag.c
            x86_vc/x86zigzag.h
            x86_vc/mmxloop.h
            x86_vc/mmxidct.c
            x86_vc/x86cpu.c
    )
else()
    target_sources(theora PRIVATE
            x86/x86int.h
            x86/x86state.c
            x86/mmxstate.c
            x86/x86enc.h
            x86/sse2trans.h
            x86/x86cpu.h
            x86/mmxfrag.c
            x86/x86zigzag.h
            x86/mmxloop.h
            x86/mmxidct.c
            x86/x86cpu.c
            x86/sse2idct.c
    )
endif()
target_include_directories(theora PUBLIC .)
target_link_libraries(theora PUBLIC ogg)
set_property(TARGET theora PROPERTY C_STANDARD 17)
set_property(TARGET theora PROPERTY LANGUAGE CXX)
