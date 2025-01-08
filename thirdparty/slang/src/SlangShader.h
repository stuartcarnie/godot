//
// Created by Stuart Carnie on 5/8/2024.
//

#ifndef GODOT_SLANGSHADER_H
#define GODOT_SLANGSHADER_H

#include "ShaderPass.h"
#include "compiled.h"

#include <filesystem>
#include <map>
#include <vector>

namespace slang {

namespace fs = std::filesystem;

struct ShaderLUT {
	fs::path path;
	std::u32string name;
	compiled::Filter filter;
	compiled::Wrap wrap_mode;
	bool is_mipmap;

	ShaderLUT() {};
	ShaderLUT(fs::path const &p_path, ShaderTextureModel const &p_spec) :
			path(p_path) {
		name = p_spec.name;
		filter = compiled::filter_from_bool(p_spec.linear);
		wrap_mode = compiled::wrap_mode_from_string(p_spec.wrap_mode);
		is_mipmap = p_spec.mipmap_input.value_or(false);
	}
};

class SlangShader;
using SlangShaderRef = std::shared_ptr<SlangShader>;

class SlangShader {
	std::map<std::u32string, ShaderParameter> _parameter_map;

public:
	std::vector<ShaderPass> passes;
	std::vector<ShaderParameter> parameters;
	std::vector<ShaderLUT> luts;

	static result::Result<SlangShaderRef> create(fs::path const &p_path);
};

} // namespace slang

#endif //GODOT_SLANGSHADER_H
