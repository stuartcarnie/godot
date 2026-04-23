//
// Created by Stuart Carnie on 7/8/2024.
//

#pragma once

#include "SlangShader.h"
#include "error.h"
#include "shader_pass_reflection.h"
#include "shader_symbols.h"

#include <spirv_glsl.hpp>

namespace slang {

enum class Limits {
	MAX_TEXTURE_BINDINGS = 16,
};

struct ShaderCompilerOptions {
};

using Compiler = std::shared_ptr<spirv_cross::Compiler>;

class ShaderPassCompiler {
	std::shared_ptr<SlangShader> shader;

	enum class ShaderType {
		VERTEX,
		FRAGMENT,
	};

	static result::Result<std::vector<uint32_t>> ir_for_pass(ShaderPass &p_pass, ShaderType type, const ShaderCompilerOptions &p_options);
	static result::Result<std::tuple<Compiler, Compiler>> make_compilers_for_pass(ShaderPass &p_pass, const ShaderCompilerOptions &p_options);
	result::Result<compiled::ShaderPass> compile_pass(ShaderPass &p_pass, const ShaderCompilerOptions &p_options) const;
	result::Result<ShaderSymbols> make_symbols() const;
	static result::Result<ShaderPassReflection> reflect(int p_pass_number, ShaderSymbols &p_sym, Compiler p_vert, Compiler p_frag);
	static bool validate_resources(const spirv_cross::ShaderResources &p_vert, const spirv_cross::ShaderResources &p_frag);
	static error::ErrorOpt add_active_buffer_ranges(ShaderPassReflection &p_ref, ShaderSymbols &p_sym, Compiler p_comp, spirv_cross::Resource &p_res, bool p_ubo);
	static compiled::UBOBufferDescriptor make_ubo_descriptor(ShaderPassReflection &p_ref);
	static compiled::PushBufferDescriptor make_push_descriptor(ShaderPassReflection &p_ref);
	static std::vector<compiled::BufferUniformDescriptor> make_descriptors(const slang::ShaderPassReflection &p_ref, ShaderBufferSemanticMeta::OffsetType p_type);
	std::vector<compiled::TextureDescriptor> make_textures(const ShaderPassReflection &p_ref, ShaderSymbols &p_sym) const;

public:
	ShaderPassCompiler(const std::shared_ptr<SlangShader> &p_shader) :
			shader(p_shader) {}

	result::Result<compiled::Shader> compile(ShaderCompilerOptions const &p_options) const;
};

} //namespace slang
