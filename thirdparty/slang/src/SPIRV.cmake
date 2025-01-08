add_library(spirv-cross STATIC)
target_sources(spirv-cross PRIVATE
        ../../spirv-cross/GLSL.std.450.h
        ../../spirv-cross/include/spirv_cross/barrier.hpp
        ../../spirv-cross/include/spirv_cross/external_interface.h
        ../../spirv-cross/include/spirv_cross/image.hpp
        ../../spirv-cross/include/spirv_cross/internal_interface.hpp
        ../../spirv-cross/include/spirv_cross/sampler.hpp
        ../../spirv-cross/include/spirv_cross/thread_group.hpp
        ../../spirv-cross/spirv.hpp
        ../../spirv-cross/spirv_cfg.cpp
        ../../spirv-cross/spirv_cfg.hpp
        ../../spirv-cross/spirv_common.hpp
        ../../spirv-cross/spirv_cross.cpp
        ../../spirv-cross/spirv_cross.hpp
        ../../spirv-cross/spirv_cross_containers.hpp
        ../../spirv-cross/spirv_cross_error_handling.hpp
        ../../spirv-cross/spirv_cross_parsed_ir.cpp
        ../../spirv-cross/spirv_cross_parsed_ir.hpp
        ../../spirv-cross/spirv_cross_util.cpp
        ../../spirv-cross/spirv_cross_util.hpp
        ../../spirv-cross/spirv_glsl.cpp
        ../../spirv-cross/spirv_glsl.hpp
        ../../spirv-cross/spirv_msl.cpp
        ../../spirv-cross/spirv_msl.hpp
        ../../spirv-cross/spirv_parser.cpp
        ../../spirv-cross/spirv_parser.hpp
        ../../spirv-cross/spirv_reflect.cpp
        ../../spirv-cross/spirv_reflect.hpp
)
target_include_directories(spirv-cross PUBLIC ../../spirv-cross)

target_compile_options(spirv-cross PRIVATE $<$<CONFIG:Debug>:-O2>)
