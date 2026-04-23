//
// Created by Stuart Carnie on 13/8/2024.
//

#pragma once

#include "error.h"
#include "shader_reflection.h"

#include <map>
#include <optional>

namespace slang {

struct ShaderPassReflection {
	using STS = compiled::ShaderTextureSemantic;
	using SBS = compiled::ShaderBufferSemantic;

	std::map<STS, std::map<int, ShaderTextureSemanticMetaRef>> textures = {
		{ STS::ORIGINAL,         {} },
		{ STS::SOURCE,           {} },
		{ STS::ORIGINAL_HISTORY, {} },
		{ STS::PASS_OUTPUT,      {} },
		{ STS::PASS_FEEDBACK,    {} },
		{ STS::USER,             {} },
	};

	std::map<SBS, std::map<int, ShaderBufferSemanticMetaRef>> texture_uniforms = {
		{ SBS::ORIGINAL_SIZE,         {} },
		{ SBS::SOURCE_SIZE,           {} },
		{ SBS::ORIGINAL_HISTORY_SIZE, {} },
		{ SBS::PASS_OUTPUT_SIZE,      {} },
		{ SBS::PASS_FEEDBACK_SIZE,    {} },
		{ SBS::USER_SIZE,             {} },
	};

	std::map<SBS, ShaderBufferSemanticMetaRef> semantics = {
		{ SBS::MVP,                 ShaderBufferSemanticMeta::create(SBS::MVP)                 },
		{ SBS::OUTPUT_SIZE,         ShaderBufferSemanticMeta::create(SBS::OUTPUT_SIZE)         },
		{ SBS::FINAL_VIEWPORT_SIZE, ShaderBufferSemanticMeta::create(SBS::FINAL_VIEWPORT_SIZE) },
		{ SBS::FRAME_COUNT,         ShaderBufferSemanticMeta::create(SBS::FRAME_COUNT)         },
		{ SBS::FRAME_DIRECTION,     ShaderBufferSemanticMeta::create(SBS::FRAME_DIRECTION)     },
	};

	std::map<int, ShaderBufferSemanticMetaRef> float_parameters;

	int pass_number;
	UBOBindingDescriptor ubo;
	PushBindingDescriptor push;

	/**
	 * Set the offset in a UBO or Push Constant for a float parameter
	 * @param p_offset
	 * @param vec_size
	 * @param p_index
	 * @param p_name
	 * @param p_ubo
	 * @return
	 */
	error::ErrorOpt set_offset(int p_offset, int vec_size, int p_index, std::string const &p_name, bool p_ubo);

	/**
	 * Set a global semantic, such as MVP
	 * @param p_offset
	 * @param vec_size
	 * @param p_semantic
	 * @param p_ubo
	 * @return
	 */
	error::ErrorOpt set_offset(int p_offset, int vec_size, SBS p_semantic, bool p_ubo);

	/**
	 * Set a size semantic for a texture
	 * @param p_offset
	 * @param p_semantic
	 * @param p_index
	 * @param p_name
	 * @param p_ubo
	 * @return
	 */
	error::ErrorOpt set_offset(int p_offset, SBS p_semantic, int p_index, std::string const &p_name, bool p_ubo);

	error::ErrorOpt set_binding(int p_binding, STS p_semantic, int p_index, std::string const &p_name);

	void to_stream(std::ostream &oss) const;

	ShaderPassReflection(int p_pass_number) :
			pass_number(p_pass_number) {}
};

} //namespace slang
