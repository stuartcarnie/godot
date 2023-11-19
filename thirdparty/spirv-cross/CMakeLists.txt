godot_add_library(spirv-cross FOLDER thirdparty NO_INCLUDES STATIC)
target_sources(spirv-cross PRIVATE
		GLSL.std.450.h
		include/spirv_cross/barrier.hpp
		include/spirv_cross/external_interface.h
		include/spirv_cross/image.hpp
		include/spirv_cross/internal_interface.hpp
		include/spirv_cross/sampler.hpp
		include/spirv_cross/thread_group.hpp
		spirv.h
		spirv.hpp
		spirv_cfg.cpp
		spirv_cfg.hpp
		spirv_common.hpp
		spirv_cpp.cpp
		spirv_cpp.hpp
		spirv_cross.cpp
		spirv_cross.hpp
		spirv_cross_containers.hpp
		spirv_cross_error_handling.hpp
		spirv_cross_parsed_ir.cpp
		spirv_cross_parsed_ir.hpp
		spirv_cross_util.cpp
		spirv_cross_util.hpp
		spirv_glsl.cpp
		spirv_glsl.hpp
		spirv_msl.cpp
		spirv_msl.hpp
		spirv_parser.cpp
		spirv_parser.hpp
		spirv_reflect.cpp
		spirv_reflect.hpp
)
target_include_directories(spirv-cross PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(spirv-cross PRIVATE -fexceptions)