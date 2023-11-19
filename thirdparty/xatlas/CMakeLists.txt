godot_add_library(xatlas FOLDER thirdparty STATIC)
target_sources(xatlas PRIVATE
        xatlas.h
        xatlas.cpp
)
target_include_directories(xatlas PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
