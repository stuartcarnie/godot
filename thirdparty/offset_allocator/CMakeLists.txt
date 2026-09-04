godot_add_library(offset-allocator FOLDER thirdparty NO_INCLUDES STATIC)
target_sources(offset-allocator PRIVATE
        offsetAllocator.cpp
        offsetAllocator.hpp
)
target_include_directories(offset-allocator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(offset-allocator PRIVATE cxx_std_20)
#target_compile_definitions(offset-allocator PRIVATE USE_16_BIT_NODE_INDICES)
