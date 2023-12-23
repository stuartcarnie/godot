godot_add_library(driver-metal FOLDER drivers STATIC)
# [[[cog
#   import cog
#   from cmake_builders import list_files
#   cog.outl('target_sources(driver-metal PRIVATE')
#   for file in list_files(cog, '.'):
#       cog.outl(f'        {file}')
#   cog.outl(')')
# ]]]
target_sources(driver-metal PRIVATE
        metal_context.h
        metal_context.mm
        metal_device_properties.h
        metal_device_properties.mm
        metal_objects.h
        metal_objects.mm
        pixel_formats.h
        pixel_formats.mm
        rendering_device_driver_metal.h
        rendering_device_driver_metal.mm
        utils.h
)
# [[[end]]]
target_compile_features(driver-metal PRIVATE cxx_std_20)
set_target_properties(driver-metal PROPERTIES XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD c++20)
target_link_libraries(driver-metal PUBLIC godot-error godot-os spirv-cross)
target_compile_definitions(driver-metal PUBLIC METAL_ENABLED=1 RD_ENABLED=1)
target_compile_options(driver-metal PRIVATE -fexceptions)
set_source_files_properties(pixel_formats.mm PROPERTIES COMPILE_FLAGS -Wno-deprecated-declarations)
target_include_directories(driver-metal PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(tools)