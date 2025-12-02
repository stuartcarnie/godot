godot_add_library(re-spirv FOLDER thirdparty STATIC)
target_sources(re-spirv PRIVATE
        re-spirv.cpp
        re-spirv.h
)
target_include_directories(re-spirv PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(re-spirv PRIVATE spirv-headers)
