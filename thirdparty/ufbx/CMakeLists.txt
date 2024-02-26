godot_add_library(ufbx FOLDER thirdparty STATIC)
target_sources(ufbx PRIVATE
        ufbx.c
        ufbx.h
)
target_compile_definitions(ufbx PRIVATE
        UFBX_NO_SUBDIVISION
        UFBX_NO_TESSELLATION
        UFBX_NO_GEOMETRY_CACHE
        UFBX_NO_SCENE_EVALUATION
        UFBX_NO_INDEX_GENERATION
        UFBX_NO_SKINNING_EVALUATION
        UFBX_NO_FORMAT_OBJ
)
target_include_directories(ufbx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
