//
// Created by Stuart Carnie on 1/1/2025.
//

#include "shader_container.h"
#include <filesystem>
#include <string>
#include <SlangShader.h>
#include <ShaderPassCompiler.h>

namespace fs = std::filesystem;

std::optional<FileShaderContainer> FileShaderContainer::create(const String &p_path) {
	fs::path path = p_path.utf8().get_data();

	SlangShaderRef ref;
	if (auto res = SlangShader::create(path); res.is_err()) {
		print_error(vformat("Failed to load shader %s : %s", p_path, String(res.take_err().to_string().c_str())));
		return std::nullopt;
	} else {
		ref = res.take();
	}

	ShaderPassCompiler compiler(ref);
	auto res = compiler.compile(ShaderCompilerOptions());
	if (res.is_err()) {
		print_error(vformat("Failed to compile shader %s : %s", p_path, String(res.take_err().to_string().c_str())));
		return std::nullopt;
	}

	return FileShaderContainer(res.take());
}

FileShaderContainer::~FileShaderContainer() {
}
