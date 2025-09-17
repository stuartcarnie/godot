godot_add_library(etcpak FOLDER thirdparty STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(etcpak PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(etcpak PRIVATE
        DecodeRGB.cpp
        DecodeRGB.hpp
        Dither.cpp
        Dither.hpp
        ForceInline.hpp
        Math.hpp
        ProcessCommon.hpp
        ProcessDxtc.cpp
        ProcessDxtc.hpp
        ProcessRGB.cpp
        ProcessRGB.hpp
        Tables.cpp
        Tables.hpp
        Vector.hpp
)
# [[[end]]]
target_include_directories(etcpak PUBLIC .)
target_compile_options(etcpak PRIVATE $<$<COMPILE_LANGUAGE:CXX,C,OBJC,OBJCXX>:$<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>>)
