//
// Created by Stuart Carnie on 5/8/2024.
//

#include "SlangShader.h"
#include <filesystem>

namespace slang {

namespace fs = std::filesystem;

result::Result<SlangShaderRef> SlangShader::create(const fs::path &p_path) {
	ShaderModel model;
	auto res = model.read(p_path);
	if (res.has_value()) {
		return res.value();
	}

	fs::path base_path = p_path.parent_path();

	SlangShader shader;

	for (int i = 0; i < model._passes.size(); i++) {
		const ShaderPassModel &shader_pass = model._passes[i];
		fs::path shader_path = shader_pass.shader;
		if (shader_path.is_relative()) {
			shader_path = canonical(base_path / shader_path);
		}
		ShaderPass pass = ShaderPass(shader_path, shader_pass);
		shader.passes.push_back(pass);
	}

	for (auto &spec : model._textures) {
		fs::path lut_path = spec.path;
		if (lut_path.is_relative()) {
			lut_path = canonical(base_path / lut_path);
		}
		shader.luts.push_back(ShaderLUT(lut_path, spec));
	}

	for (auto &pass : shader.passes) {
		for (auto &param : pass._parser.parameters) {
			if (const auto &it = shader._parameter_map.find(param.name); it != shader._parameter_map.end()) {
				if (it->second != param) {
					return error::Error::parameter_mismatch(param.name);
				}
				continue;
			}
			shader._parameter_map[param.name] = param;
			shader.parameters.push_back(param);
		}
	}

	return std::make_shared<SlangShader>(shader);
}

}
