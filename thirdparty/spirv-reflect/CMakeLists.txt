godot_add_library(spirv-reflect FOLDER thirdparty STATIC)
target_sources(spirv-reflect PRIVATE
        spirv_reflect.c
        spirv_reflect.h
)
target_include_directories(spirv-reflect PUBLIC include)
