godot_add_library(metal FOLDER drivers STATIC)
add_subdirectory(tools)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(metal PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(metal PRIVATE
        inflection_map.h
        metal3_objects.cpp
        metal3_objects.h
        metal4_objects.cpp
        metal4_objects.h
        metal_allocator.cpp
        metal_allocator.h
        metal_device_profile.cpp
        metal_device_profile.h
        metal_device_properties.cpp
        metal_device_properties.h
        metal_objects_shared.cpp
        metal_objects_shared.h
        metal_utils.h
        pixel_formats.cpp
        pixel_formats.h
        rendering_context_driver_metal.cpp
        rendering_context_driver_metal.h
        rendering_device_driver_metal.cpp
        rendering_device_driver_metal.h
        rendering_device_driver_metal3.cpp
        rendering_device_driver_metal3.h
        rendering_device_driver_metal4.cpp
        rendering_device_driver_metal4.h
        rendering_shader_container_metal.cpp
        rendering_shader_container_metal.h
        sha256_digest.h
)
# [[[end]]]
target_compile_features(metal PRIVATE cxx_std_20 c_std_11)
set_target_properties(metal PROPERTIES
        XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD c++20
        C_STANDARD 11
        C_STANDARD_REQUIRED OFF
        C_EXTENSIONS ON
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
)
target_link_libraries(metal PUBLIC core spirv-cross metal-cpp offset-allocator)
target_compile_definitions(metal
        PUBLIC
        METAL_ENABLED
        RD_ENABLED
        PRIVATE
        METAL4_ENABLED
)
target_compile_options(metal PRIVATE -fexceptions -Wno-c99-designator)
# disable -Wunused-variable for now, as it's too noisy
target_compile_options(metal PRIVATE -Wno-unused-variable)
target_include_directories(metal PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# add_subdirectory(tools)

group_target_sources(metal)
