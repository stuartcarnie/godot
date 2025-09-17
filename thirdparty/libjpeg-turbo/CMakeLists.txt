godot_add_library(jpeg-turbo FOLDER thirdparty STATIC)

set(JPEG_SOURCES_COMMON
        src/jaricom.c src/jcapimin.c src/jcarith.c src/jchuff.c src/jcicc.c src/jcinit.c src/jcmarker.c
        src/jcmaster.c src/jcomapi.c src/jcparam.c src/jcphuff.c src/jctrans.c src/jdapimin.c src/jdarith.c
        src/jdatadst.c src/jdatadst-tj.c src/jdatasrc.c src/jdatasrc-tj.c src/jdhuff.c src/jdicc.c src/jdinput.c
        src/jdmarker.c src/jdmaster.c src/jdphuff.c src/jdtrans.c src/jerror.c src/jfdctflt.c src/jmemmgr.c
        src/jmemnobs.c src/jpeg_nbits.c src/transupp.c src/turbojpeg.c)

set(JPEG_SOURCES_BIT
        src/jcapistd.c src/jccoefct.c src/jccolor.c src/jcdctmgr.c src/jcmainct.c src/jcprepct.c src/jcsample.c
        src/jdcoefct.c src/jdcolor.c src/jdapistd.c src/jddctmgr.c src/jdmainct.c src/jdmerge.c src/jdpostct.c
        src/jdsample.c src/jfdctfst.c src/jfdctint.c src/jidctflt.c src/jidctfst.c src/jidctint.c src/jidctred.c
        src/jutils.c src/jquant1.c src/jquant2.c)

add_library(jpeg12 OBJECT ${JPEG_SOURCES_BIT})
target_compile_definitions(jpeg12 PRIVATE BITS_IN_JSAMPLE=12)
target_compile_options(jpeg12 PUBLIC $<$<COMPILE_LANGUAGE:CXX,C,OBJC,OBJCXX>:$<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>>)

add_library(jpeg8 OBJECT ${JPEG_SOURCES_BIT})
target_compile_definitions(jpeg8 PRIVATE BITS_IN_JSAMPLE=8)
target_compile_options(jpeg8 PUBLIC $<$<COMPILE_LANGUAGE:CXX,C,OBJC,OBJCXX>:$<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>>)

target_sources(jpeg-turbo PUBLIC ${JPEG_SOURCES_COMMON})
target_link_libraries(jpeg-turbo PUBLIC jpeg12 jpeg8)
target_compile_options(jpeg-turbo PUBLIC $<$<COMPILE_LANGUAGE:CXX,C,OBJC,OBJCXX>:$<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>>)
target_include_directories(jpeg-turbo PUBLIC src)
