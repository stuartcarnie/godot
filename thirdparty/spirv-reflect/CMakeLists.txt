godot_add_library(spirv-reflect FOLDER thirdparty STATIC)
target_sources(spirv-reflect PRIVATE
        include/spirv/unified1/spirv.h
        spirv_reflect.c
        spirv_reflect.h
)
target_include_directories(spirv-reflect PUBLIC include)
