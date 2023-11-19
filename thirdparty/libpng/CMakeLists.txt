godot_add_library(png FOLDER thirdparty STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(png PRIVATE')
#   for file in list_files(cog, ['.', 'arm']):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(png PRIVATE
        arm/arm_init.c
        arm/filter_neon_intrinsics.c
        arm/palette_neon_intrinsics.c
        png.c
        png.h
        pngconf.h
        pngdebug.h
        pngerror.c
        pngget.c
        pnginfo.h
        pnglibconf.h
        pngmem.c
        pngpread.c
        pngpriv.h
        pngread.c
        pngrio.c
        pngrtran.c
        pngrutil.c
        pngset.c
        pngstruct.h
        pngtrans.c
        pngwio.c
        pngwrite.c
        pngwtran.c
        pngwutil.c
)
# [[[end]]]
target_include_directories(png PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(png PUBLIC zlib)
