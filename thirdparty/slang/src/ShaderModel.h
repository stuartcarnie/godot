//
// Created by Stuart Carnie on 5/8/2024.
//

#pragma once

#include "error.h"
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace slang {

namespace fs = std::filesystem;
using std::u32string, std::vector, std::optional;

enum class ScaleAxis {
	X,
	Y,
};

struct ShaderPassModel {
	int pass;
	fs::path shader;
	optional<u32string> wrap_mode;
	optional<u32string> alias;
	optional<u32string> scale_type;
	optional<u32string> scale_type_x;
	optional<u32string> scale_type_y;

	optional<bool> filter_linear;
	optional<bool> srgb_framebuffer;
	optional<bool> float_framebuffer;
	optional<bool> mipmap_input;

	optional<uint32_t> frame_count_mod;

	optional<double> scale;
	optional<double> scale_x;
	optional<double> scale_y;

	const optional<u32string> scale_type_for_axis(ScaleAxis a) const {
		switch (a) {
			case ScaleAxis::X:
				return scale_type_x;
			case ScaleAxis::Y:
				return scale_type_y;
		}
	}

	optional<double> scale_value(ScaleAxis a) const {
		switch (a) {
			case ScaleAxis::X:
				return scale_x;
			case ScaleAxis::Y:
				return scale_y;
		}
	}

	ShaderPassModel() : pass(0) {}
	ShaderPassModel(int p_pass, fs::path const &p_shader) :
			pass(p_pass), shader(p_shader) {}
	~ShaderPassModel() {}
};

struct ShaderTextureModel {
	u32string name;
	u32string path;
	optional<u32string> wrap_mode;
	optional<bool> linear;
	optional<bool> mipmap_input;

	ShaderTextureModel() {};
	ShaderTextureModel(u32string p_name, u32string p_path) :
			name(p_name), path(p_path) {}
};

struct ShaderParameterModel {
	u32string name;
	double value;

	ShaderParameterModel() : value(0) {}
	ShaderParameterModel(const u32string &p_name, double p_value) :
			name(p_name), value(p_value) {}
};

struct ShaderModel {
	vector<ShaderPassModel> _passes;
	vector<ShaderTextureModel> _textures;
	vector<ShaderParameterModel> _parameters;

	error::ErrorOpt read(const fs::path &p_path);

	ShaderModel() {}
	~ShaderModel() {}
};

} // namespace slang
