//
// Created by Stuart Carnie on 12/8/2024.
//

#pragma once

#include "error.h"
#include "shader_reflection.h"

#include <map>
#include <set>

namespace slang {

class ShaderSymbols {
	std::map<std::string, ShaderBufferSemanticMapRef> float_parameter_semantic_map;
	std::map<std::string, ShaderTextureSemanticMapRef> texture_semantic_map;
	std::map<std::string, ShaderBufferSemanticMapRef> texture_uniform_semantic_map;

	// static

	static std::set<compiled::ShaderTextureSemantic> texture_semantic_arrays;
	static std::set<compiled::ShaderBufferSemantic> uniform_semantic_arrays;
	static std::map<std::string, compiled::ShaderTextureSemantic> texture_semantic_names;
	static std::map<std::string, compiled::ShaderBufferSemantic> texture_semantic_uniform_names;
	static std::map<std::string, ShaderBufferSemanticMapRef> semantic_uniform_names;

	optional<ShaderBufferSemanticMapRef> find_texture_semantic_for_uniform_name(const std::string &p_name) const;
	optional<ShaderTextureSemanticMapRef> find_texture_semantic_for_name(const std::string &p_name) const;

public:
	error::ErrorOpt add_texture_semantic(compiled::ShaderTextureSemantic p_semantic, int p_index, const std::string &p_name);
	error::ErrorOpt add_texture_buffer_semantic(compiled::ShaderBufferSemantic p_semantic, int p_index, const std::string &p_name);
	error::ErrorOpt add_float_parameter_semantic(int p_index, const std::string &p_name);

	optional<ShaderBufferSemanticMapRef> get_buffer_semantic_for_uniform(const std::string &p_name) const;
	bool texture_semantic_is_array(compiled::ShaderTextureSemantic p_semantic) const {
		return texture_semantic_arrays.find(p_semantic) != texture_semantic_arrays.end();
	};
	optional<ShaderBufferSemanticMapRef> get_texture_semantic_for_uniform_name(const std::string &p_name) const;
	optional<ShaderTextureSemanticMapRef> get_texture_semantic_for_name(const std::string &p_name) const;
};

} //namespace slang
