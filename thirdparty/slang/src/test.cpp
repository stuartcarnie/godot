//
// Created by Stuart Carnie on 6/8/2024.
//

#include "ShaderPassCompiler.h"
#include "SlangShader.h"
#include "error.h"

#include "thirdparty/cxxopts/include/cxxopts.hpp"
#include "u32string.h"
#include <glslang/Public/ShaderLang.h>
#include <iostream>
#include <string>

int main(int argc, char **argv) {
	cxxopts::Options options("test", "Validate shader compilation");
	// clang-format off
	options.add_options()
			("h,help", "Print usage")
			("filename", "The filename to process", cxxopts::value<std::string>())
	;
	// clang-format on
	options.parse_positional({ "filename" });
	auto result = options.parse(argc, argv);

	auto filename = result["filename"];
	if (filename.count() == 0) {
		printf("No filename provided\n");
		return 1;
	}

	SlangShaderRef shader;
	if (auto res = SlangShader::create(filename.as<std::string>()); res.is_err()) {
		std::cout << "Failed to load shader " << filename.as<std::string>()
				  << " : " << res.take_err().to_string() << std::endl;
		printf("%s\n", res.take_err().to_string().c_str());
		return 1;
	} else {
		shader = res.take();
	}

	glslang::InitializeProcess();

	ShaderPassCompiler compiler(shader);

	auto res = compiler.compile(ShaderCompilerOptions());
	if (res.is_err()) {
		auto err = res.take_err();
		std::cout << "Failed to compile shader: "
				  << filename.as<std::string>()
				  << " : " << err.to_string() << std::endl;
		return 1;
	}

	auto compiled_shader = res.take();
	std::cout << "Compiled shader: " << std::endl
			  << "  pass_count      : " << compiled_shader.passes.size() << std::endl
			  << "  parameter_count : " << compiled_shader.parameters.size() << std::endl
			  << "  lut_count       : " << compiled_shader.luts.size() << std::endl
			  << "  history_count   : " << compiled_shader.history_count << std::endl;

	glslang::FinalizeProcess();

	return 0;
}
