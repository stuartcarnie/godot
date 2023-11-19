godot_add_library(tinyexr FOLDER thirdparty STATIC)
target_sources(tinyexr PRIVATE
        tinyexr.h
        tinyexr.cc
)
target_compile_definitions(tinyexr PRIVATE -DTINYEXR_USE_MINIZ=0)
target_include_directories(tinyexr PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tinyexr PRIVATE zlib)
