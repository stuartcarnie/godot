//
// Created by Stuart Carnie on 12/8/2024.
//

#include "shader_symbols.h"
#include <spirv_cross.hpp>

namespace slang {

using std::map, std::set, std::make_shared, std::make_pair, spirv_cross::SPIRType;

using STS = compiled::ShaderTextureSemantic;
using SBS = compiled::ShaderBufferSemantic;

set<STS> ShaderSymbols::texture_semantic_arrays = {
	STS::ORIGINAL_HISTORY,
	STS::PASS_OUTPUT,
	STS::PASS_FEEDBACK,
	STS::USER,
};

set<SBS> ShaderSymbols::uniform_semantic_arrays = {
	SBS::ORIGINAL_HISTORY_SIZE,
	SBS::PASS_OUTPUT_SIZE,
	SBS::PASS_FEEDBACK_SIZE,
	SBS::USER_SIZE,
};

map<std::string, STS> ShaderSymbols::texture_semantic_names = {
	{ "Original",        STS::ORIGINAL         },
	{ "Source",          STS::SOURCE           },
	{ "OriginalHistory", STS::ORIGINAL_HISTORY },
	{ "PassOutput",      STS::PASS_OUTPUT      },
	{ "PassFeedback",    STS::PASS_FEEDBACK    },
	{ "User",            STS::USER             },
};

map<std::string, SBS> ShaderSymbols::texture_semantic_uniform_names = {
	{ "OriginalSize",        SBS::ORIGINAL_SIZE         },
	{ "SourceSize",          SBS::SOURCE_SIZE           },
	{ "OriginalHistorySize", SBS::ORIGINAL_HISTORY_SIZE },
	{ "PassOutputSize",      SBS::PASS_OUTPUT_SIZE      },
	{ "PassFeedbackSize",    SBS::PASS_FEEDBACK_SIZE    },
	{ "UserSize",            SBS::USER_SIZE             },
};

map<std::string, ShaderBufferSemanticMapRef> ShaderSymbols::semantic_uniform_names = {
	{ "MVP",			   ShaderBufferSemanticMap::create(SBS::MVP,                 SPIRType::Float, 4, 4) },
	{ "OutputSize",        ShaderBufferSemanticMap::create(SBS::OUTPUT_SIZE,         SPIRType::Float, 4, 1) },
	{ "FinalViewportSize", ShaderBufferSemanticMap::create(SBS::FINAL_VIEWPORT_SIZE, SPIRType::Float, 4, 1) },
	{ "FrameCount",        ShaderBufferSemanticMap::create(SBS::FRAME_COUNT,         SPIRType::UInt,  1, 1) },
	{ "FrameDirection",    ShaderBufferSemanticMap::create(SBS::FRAME_DIRECTION,     SPIRType::Int,   1, 1) },
};

error::ErrorOpt ShaderSymbols::add_texture_semantic(compiled::ShaderTextureSemantic p_semantic, int p_index, const std::string &p_name) {
	if (texture_semantic_map.contains(p_name)) {
		char *raw_str = nullptr;
		asprintf(&raw_str, "pass %d: alias %s already exists for texture semantic %s", p_index, p_name.c_str(), compiled::to_cstr(p_semantic));
		std::unique_ptr<char, decltype(&free)> str(raw_str, free);
		return error::Error::failed(str.get());
	}

	texture_semantic_map[p_name] = ShaderTextureSemanticMap::create(p_semantic, p_index, p_name);

	return {};
}

error::ErrorOpt ShaderSymbols::add_texture_buffer_semantic(compiled::ShaderBufferSemantic p_semantic, int p_index, const std::string &p_name) {
	if (texture_uniform_semantic_map.contains(p_name)) {
		char *raw_str = nullptr;
		asprintf(&raw_str, "pass %d: alias %s already exists for texture buffer semantic %s", p_index, p_name.c_str(), compiled::to_cstr(p_semantic));
		std::unique_ptr<char, decltype(&free)> str(raw_str, free);
		return error::Error::failed(str.get());
	}
	texture_uniform_semantic_map[p_name] = ShaderBufferSemanticMap::create(p_semantic, p_index, p_name, spirv_cross::SPIRType::BaseType::Float, 4, 1);

	return {};
}

error::ErrorOpt ShaderSymbols::add_float_parameter_semantic(int p_index, const std::string &p_name) {
	if (float_parameter_semantic_map.contains(p_name)) {
		char *raw_str = nullptr;
		asprintf(&raw_str, "pass %d: float parameter %s already exists", p_index, p_name.c_str());
		std::unique_ptr<char, decltype(&free)> str(raw_str, free);
		return error::Error::failed(str.get());
	}

	float_parameter_semantic_map[p_name] = ShaderBufferSemanticMap::create(SBS::FLOAT_PARAMETER, p_index, p_name, spirv_cross::SPIRType::BaseType::Float, 1, 1);

	return {};
}

optional<ShaderBufferSemanticMapRef> ShaderSymbols::get_buffer_semantic_for_uniform(const std::string &p_name) const {
	if (auto it = float_parameter_semantic_map.find(p_name); it != float_parameter_semantic_map.end()) {
		return it->second;
	}

	if (auto it = semantic_uniform_names.find(p_name); it != semantic_uniform_names.end()) {
		return it->second;
	}

	return std::nullopt;
}

optional<ShaderBufferSemanticMapRef> ShaderSymbols::get_texture_semantic_for_uniform_name(const std::string &p_uniform_name) const {
	if (auto it = texture_uniform_semantic_map.find(p_uniform_name); it != texture_uniform_semantic_map.end()) {
		return it->second;
	}

	return find_texture_semantic_for_uniform_name(p_uniform_name);
}

optional<ShaderTextureSemanticMapRef> ShaderSymbols::get_texture_semantic_for_name(const std::string &p_name) const {
	if (auto it = texture_semantic_map.find(p_name); it != texture_semantic_map.end()) {
		return it->second;
	}

	return find_texture_semantic_for_name(p_name);
}

optional<ShaderBufferSemanticMapRef> ShaderSymbols::find_texture_semantic_for_uniform_name(const std::string &p_name) const {
	for (auto &it : texture_semantic_uniform_names) {
		if (uniform_semantic_arrays.contains(it.second)) {
			// An array texture may be referred to as PassOutput0, PassOutput1, etc
			// if p_name starts with the semantic name, then it's a match
			if (p_name.rfind(it.first, 0) == 0) {
				// TODO: Validate the suffix is a number and within range
				int index = atoi(p_name.substr(it.first.size()).c_str());
				return ShaderBufferSemanticMap::create(it.second, index, p_name, spirv_cross::SPIRType::BaseType::Float, 4, 1);
			}
		} else if (p_name == it.first) {
			return ShaderBufferSemanticMap::create(it.second, 0, p_name, spirv_cross::SPIRType::BaseType::Float, 4, 1);
		}
	}
	return std::nullopt;
}

optional<ShaderTextureSemanticMapRef> ShaderSymbols::find_texture_semantic_for_name(const std::string &p_name) const {
	for (auto &it : texture_semantic_names) {
		if (texture_semantic_arrays.contains(it.second)) {
			// An array texture may be referred to as PassOutput0, PassOutput1, etc
			// if p_name starts with the semantic name, then it's a match
			if (p_name.rfind(it.first, 0) == 0) {
				// TODO: Validate the suffix is a number and within range
				int index = atoi(p_name.substr(it.first.size()).c_str());
				return ShaderTextureSemanticMap::create(it.second, index, p_name);
			}
		} else if (p_name == it.first) {
			return ShaderTextureSemanticMap::create(it.second, 0, p_name);
		}
	}
	return std::nullopt;
}

} //namespace slang
