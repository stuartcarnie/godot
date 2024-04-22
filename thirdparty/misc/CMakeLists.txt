godot_add_library(godot-thirdparty-misc FOLDER thirdparty STATIC)
target_sources(godot-thirdparty-misc PRIVATE
        cubemap_coeffs.h
        fastlz.c
        fastlz.h
        # ifaddrs-android.cc # Disabled until we support Android
        # ifaddrs-android.h # Disabled until we support Android
        mikktspace.c
        mikktspace.h
        ok_color.h
        ok_color_shader.h
        pcg.cpp
        pcg.h
        polypartition.cpp
        polypartition.h
        r128.c
        r128.h
        smaz.c
        smaz.h
        smolv.cpp
        smolv.h
        stb_rect_pack.h
        yuv2rgb.h
)
target_compile_options(godot-thirdparty-misc PRIVATE $<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>)
# HACK Only for linux
target_include_directories(godot-thirdparty-misc PRIVATE ${GODOT_ENGINE_ROOT_DIRECTORY} ${GODOT_ENGINE_ROOT_DIRECTORY}/platform/linuxbsd)
