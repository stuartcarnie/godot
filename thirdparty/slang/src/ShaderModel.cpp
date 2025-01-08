//
// Created by Stuart Carnie on 5/8/2024.
//

#include "ShaderModel.h"
#include "scanner.h"
#include "thirdparty/simdutf/simdutf.h"
#include "u32string.h"

#include <icu4c/common/unicode/uchar.h>

#include <fstream>
#include <iostream>
#include <map>

namespace slang {

static error::ErrorOpt read_config_file(const fs::path &p_path, std::map<u32string, u32string> &r_data) {
	std::fstream file(p_path);
	if (!file.is_open()) {
		return error::Error::path_not_found(p_path);
	}

	std::string line;
	while (std::getline(file, line)) {
		u32string str = u32::from_utf8(line);
		if (str.empty()) {
			continue;
		}

		char32_t const *ptr = str.data();
		char32_t const *end = ptr + str.length();

		ptr = skip_whitespace(ptr, end);
		if (ptr == end) {
			continue;
		}

		// parse key
		char32_t const *key_start = ptr;
		while (ptr != end && (u_isalnum(*ptr) || *ptr == U'_')) {
			ptr++;
		}
		u32string key(key_start, ptr - key_start);

		ptr = skip_whitespace(ptr, end);

		if (ptr == end || *ptr != U'=') {
			continue;
		}

		ptr++; // skip '='
		ptr = skip_whitespace(ptr, end);
		if (ptr == end) {
			continue;
		}

		// scan value, which could be quoted
		u32string value;
		if (*ptr == U'"') {
			value = scan_quoted_string(ptr, end);
		} else {
			char32_t const *value_start = ptr;
			while (ptr != end && !u_isspace(*ptr) && *ptr != U'#') {
				ptr++;
			}
			value = u32string(value_start, ptr - value_start);
		}

		r_data[key] = value;
	}

	return std::nullopt;
}

static std::variant<ShaderPassModel, error::Error> parse_shader_pass(int p_pass, const std::map<u32string, u32string> &p_data) {
	u32string pass_str = u32::from_int(p_pass);
	u32string key = U"shader" + pass_str;
	auto val = p_data.find(key);
	if (val == p_data.end()) {
		return error::Error::parse();
	}

	ShaderPassModel pass(p_pass, val->second);

#define SET_OPTIONAL_STRING(NAME)            \
	val = p_data.find(U## #NAME + pass_str); \
	if (val != p_data.end()) {               \
		pass.NAME = val->second;             \
	}

	SET_OPTIONAL_STRING(wrap_mode);
	SET_OPTIONAL_STRING(alias);
	SET_OPTIONAL_STRING(scale_type);
	SET_OPTIONAL_STRING(scale_type_x);
	SET_OPTIONAL_STRING(scale_type_y);

#undef SET_OPTIONAL_STRING

#define SET_OPTIONAL_BOOL(NAME)               \
	val = p_data.find(U## #NAME + pass_str);  \
	if (val != p_data.end()) {                \
		if (val->second == U"true") {         \
			pass.NAME = true;                 \
		} else if (val->second == U"false") { \
			pass.NAME = false;                \
		}                                     \
	}

	SET_OPTIONAL_BOOL(filter_linear);
	SET_OPTIONAL_BOOL(srgb_framebuffer);
	SET_OPTIONAL_BOOL(float_framebuffer);
	SET_OPTIONAL_BOOL(mipmap_input);

#undef SET_OPTIONAL_BOOL

	val = p_data.find(U"frame_count_mod" + pass_str);
	if (val != p_data.end()) {
		pass.frame_count_mod = (uint32_t)u32::to_int(val->second);
	}

#define SET_OPTIONAL_DOUBLE(NAME)                \
	val = p_data.find(U## #NAME + pass_str);     \
	if (val != p_data.end()) {                   \
		pass.NAME = u32::to_double(val->second); \
	}
	SET_OPTIONAL_DOUBLE(scale);
	SET_OPTIONAL_DOUBLE(scale_x);
	SET_OPTIONAL_DOUBLE(scale_y);

#undef SET_OPTIONAL_DOUBLE

	return pass;
}

static void parse_textures(const std::map<u32string, u32string> &p_data, std::vector<ShaderTextureModel> &r_textures) {
	auto textures_names = p_data.find(U"textures");
	if (textures_names == p_data.end()) {
		return;
	}

	vector<u32string> names = u32::split(textures_names->second, U';');
	for (u32string &name : names) {
		auto iter = p_data.find(name);
		if (iter == p_data.end()) {
			continue;
		}

		ShaderTextureModel texture(name, iter->second);

		auto val = p_data.find(name + U"_wrap_mode");
		if (val != p_data.end()) {
			texture.wrap_mode = val->second;
		}

		val = p_data.find(name + U"_linear");
		if (val != p_data.end()) {
			if (val->second == U"true") {
				texture.linear = true;
			} else if (val->second == U"false") {
				texture.linear = false;
			}
		}

		val = p_data.find(name + U"_mipmap");
		if (val != p_data.end()) {
			if (val->second == U"true") {
				texture.mipmap_input = true;
			} else if (val->second == U"false") {
				texture.mipmap_input = false;
			}
		}

		r_textures.push_back(texture);
	}

	return;
}

void parse_parameters(const std::map<u32string, u32string> &p_data, vector<ShaderParameterModel> &r_params) {
	auto param_names = p_data.find(U"parameters");
	if (param_names == p_data.end()) {
		return;
	}

	vector<u32string> names = u32::split(param_names->second, U';');
	for (u32string &name : names) {
		auto val = p_data.find(name);
		if (val == p_data.end()) {
			continue;
		}

		ShaderParameterModel param(name, u32::to_double(val->second));
		r_params.push_back(param);
	}

	return;
}

error::ErrorOpt ShaderModel::read(const fs::path &p_path) {
	std::map<u32string, u32string> data;
	{
		auto res = read_config_file(p_path, data);
		if (res.has_value()) {
			return res.value();
		}
	}

	auto val = data.find(U"shaders");
	if (val == data.end()) {
		return error::Error::parse();
	}

	int64_t shaders = u32::to_int(val->second);
	if (shaders < 1) {
		return error::Error::failed();
	}

	_passes.clear();
	_textures.clear();
	_parameters.clear();

	for (int i = 0; i < shaders; i++) {
		std::variant<ShaderPassModel, error::Error> res = parse_shader_pass(i, data);
		if (auto const err = std::get_if<error::Error>(&res); err != nullptr) {
			return *err;
		}
		ShaderPassModel pass = std::get<ShaderPassModel>(res);
		_passes.push_back(pass);
	}

	parse_textures(data, _textures);
	parse_parameters(data, _parameters);

	return std::nullopt;
}

} // namespace slang
