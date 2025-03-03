//
// Created by Stuart Carnie on 7/8/2024.
//

#include "ShaderPassCompiler.h"
#include "ShaderPass.h"
#include "u32string.h"
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <spirv_glsl.hpp>
#include <spirv_parser.hpp>

#include <iostream>
#include <set>
#include <sstream>

namespace slang {

using STS = compiled::ShaderTextureSemantic;
using SBS = compiled::ShaderBufferSemantic;

result::Result<compiled::Shader> ShaderPassCompiler::compile(const ShaderCompilerOptions &p_options) {
	std::vector<compiled::ShaderPass> passes;
	for (ShaderPass &pass : shader->passes) {
		// Compile the pass
		auto res = compile_pass(pass, p_options);
		if (res.is_err()) {
			return res.take_err();
		}
		passes.push_back(res.take());
	}

	std::set<int> feedback_passes;
	for (auto &pass : passes) {
		for (auto &tex : pass.textures) {
			if (tex.semantic == STS::PASS_FEEDBACK) {
				feedback_passes.insert(tex.index);
			}
		}
	}
	for (auto &pass_number : feedback_passes) {
		passes[pass_number].is_feedback = true;
	}

	std::vector<compiled::Parameter> parameters;
	for (int i = 0; i < shader->parameters.size(); i++) {
		auto &param = shader->parameters[i];
		parameters.push_back(compiled::Parameter{
				.index = i,
				.name = u32::to_utf8(param.name),
				.desc = u32::to_utf8(param.desc),
				.initial = param.initial,
				.minimum = param.minimum,
				.maximum = param.maximum,
				.step = param.step,
		});
	}

	std::vector<compiled::LUT> luts;
	for (int i = 0; i < shader->luts.size(); i++) {
		auto &lut = shader->luts[i];
		luts.push_back(compiled::LUT{
				.url = lut.path.string(),
				.name = u32::to_utf8(lut.name),
				.filter = lut.filter,
				.wrap_mode = lut.wrap_mode,
				.is_mipmap = lut.is_mipmap,
		});
	}

	// Find the maximum index for the OriginalHistory texture semantic to determine
	// how many frames of original history is required.
	int history_count = 0;
	for (auto &pass : passes) {
		for (auto &tex : pass.textures) {
			if (tex.semantic == STS::ORIGINAL_HISTORY) {
				history_count = std::max(history_count, tex.index + 1);
			}
		}
	}

	return compiled::Shader{
		.passes = passes,
		.parameters = parameters,
		.luts = luts,
		.history_count = history_count,
	};
}

result::Result<compiled::ShaderPass> ShaderPassCompiler::compile_pass(ShaderPass &p_pass, const ShaderCompilerOptions &p_options) {
	auto res = make_compilers_for_pass(p_pass, p_options);
	if (res.is_err()) {
		return res.take_err();
	}

	// Compile the pass
	auto compilers = res.take();

	Compiler vert_compiler = std::get<0>(compilers);
	auto vert_src_c = vert_compiler->compile();
	auto vert_src = u32::to_ascii(p_pass._parser.get_vert_source());

	Compiler frag_compiler = std::get<1>(compilers);
	auto frag_src_c = frag_compiler->compile();
	auto frag_src = u32::to_ascii(p_pass._parser.get_frag_source());

	auto sym_res = make_symbols();
	if (sym_res.is_err()) {
		return sym_res.take_err();
	}
	auto sym = sym_res.take();
	auto ref_res = reflect(p_pass.index, sym, vert_compiler, frag_compiler);
	if (ref_res.is_err()) {
		return ref_res.take_err();
	}
	auto ref = ref_res.take();

	auto ubo = make_ubo_descriptor(ref);
	auto push = make_push_descriptor(ref);
	auto textures = make_textures(ref, sym);

	return compiled::ShaderPass{
		.index = p_pass.index,
		.vertex_source = vert_src,
		.fragment_source = frag_src,
		.frame_count_mod = p_pass.frame_count_mod,
		.scale_x = p_pass.scale_x,
		.scale_y = p_pass.scale_y,
		.filter = p_pass.filter,
		.wrap_mode = p_pass.wrap_mode,
		.format = {
				   .format = p_pass._parser.format,
				   .is_float = p_pass.is_float,
				   .is_sRGB = p_pass.is_sRGB,
				   },
		.is_feedback = false,
		.ubo = ubo,
		.push = push,
		.textures = textures,
		.alias = p_pass.alias.has_value() ? std::make_optional(u32::to_utf8(p_pass.alias.value())) : std::nullopt,
	};
}

static spirv_cross::CompilerGLSL::Options default_options() {
	spirv_cross::CompilerGLSL::Options options;
	options.fragment.default_float_precision = spirv_cross::CompilerGLSL::Options::Precision::Highp;
	options.fragment.default_int_precision = spirv_cross::CompilerGLSL::Options::Precision::Highp;
	return options;
}

static Compiler compiler_from_spirv(std::vector<uint32_t> &&p_spirv) {
	spirv_cross::Parser parser(std::move(p_spirv));
	parser.parse();
	Compiler compiler = std::make_shared<spirv_cross::Compiler>(parser.get_parsed_ir());
	//	compiler->set_op
	//	compiler->set_common_options(default_options());
	return compiler;
}

result::Result<std::tuple<Compiler, Compiler>> ShaderPassCompiler::make_compilers_for_pass(ShaderPass &p_pass, const ShaderCompilerOptions &p_options) {
	auto vert_res = ir_for_pass(p_pass, ShaderType::VERTEX, p_options);
	if (vert_res.is_err()) {
		return vert_res.take_err();
	}
	auto vert_spirv = vert_res.take();
	auto vert_compiler = compiler_from_spirv(std::move(vert_spirv));

	auto frag_res = ir_for_pass(p_pass, ShaderType::FRAGMENT, p_options);
	if (frag_res.is_err()) {
		return frag_res.take_err();
	}

	auto frag_spirv = frag_res.take();
	auto frag_compiler = compiler_from_spirv(std::move(frag_spirv));

	return std::make_tuple(std::move(vert_compiler), std::move(frag_compiler));
}

result::Result<std::vector<uint32_t>> ShaderPassCompiler::ir_for_pass(ShaderPass &p_pass, ShaderType type, const ShaderCompilerOptions &p_options) {
	auto source = u32::to_ascii(type == ShaderType::VERTEX ? p_pass._parser.get_vert_source() : p_pass._parser.get_frag_source());

	EShLanguage stage = type == ShaderType::VERTEX ? EShLangVertex : EShLangFragment;
	int ClientInputSemanticsVersion = 100;
	auto ClientVersion = glslang::EShTargetVulkan_1_1;
	auto TargetVersion = glslang::EShTargetSpv_1_6;

	glslang::TShader glsl(stage);
	char const *source_cstr = source.c_str();
	glsl.setStrings(&source_cstr, 1);
	glsl.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, ClientInputSemanticsVersion);
	glsl.setEnvClient(glslang::EShClientVulkan, ClientVersion);
	glsl.setEnvTarget(glslang::EShTargetSpv, TargetVersion);

	auto messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules | EShMsgDebugInfo);

	const int DefaultVersion = 110;

	if (!glsl.parse(GetDefaultResources(), DefaultVersion, false, messages)) {
		std::string error = glsl.getInfoLog();
		error += "\n";
		error += glsl.getInfoDebugLog();
		return error::Error::glsl_compile(error);
	}

	//link
	glslang::TProgram program;
	program.addShader(&glsl);

	if (!program.link(messages)) {
		std::string error = program.getInfoLog();
		error += "\n";
		error += program.getInfoDebugLog();
		return error::Error::glsl_compile(error);
	}

	std::vector<uint32_t> spirv;
	spv::SpvBuildLogger logger;
	glslang::SpvOptions spvOptions;

	glslang::GlslangToSpv(*program.getIntermediate(stage), spirv, &logger, &spvOptions);

	return spirv;
}

result::Result<ShaderSymbols> ShaderPassCompiler::make_symbols() const {
	ShaderSymbols sym;

	// add aliases
	for (const auto &pass : shader->passes) {
		if (!pass.alias.has_value() || pass.alias->empty()) {
			continue;
		}

		auto name = u32::to_utf8(pass.alias.value());
		auto index = pass.index;

		if (auto err = sym.add_texture_semantic(STS::PASS_OUTPUT, index, name); err.has_value()) {
			return err.value();
		}
		if (auto err = sym.add_texture_buffer_semantic(SBS::PASS_OUTPUT_SIZE, index, name + "Size"); err.has_value()) {
			return err.value();
		}
		if (auto err = sym.add_texture_semantic(STS::PASS_FEEDBACK, index, name + "Feedback"); err.has_value()) {
			return err.value();
		}
		if (auto err = sym.add_texture_buffer_semantic(SBS::PASS_FEEDBACK_SIZE, index, name + "FeedbackSize"); err.has_value()) {
			return err.value();
		}
	}

	// luts
	for (int i = 0; i < shader->luts.size(); i++) {
		auto name = u32::to_utf8(shader->luts[i].name);
		if (auto err = sym.add_texture_semantic(STS::USER, i, name); err.has_value()) {
			return err.value();
		}
		if (auto err = sym.add_texture_buffer_semantic(SBS::USER_SIZE, i, name + "Size"); err.has_value()) {
			return err.value();
		}
	}

	// shader parameters
	for (int i = 0; i < shader->parameters.size(); i++) {
		auto name = u32::to_utf8(shader->parameters[i].name);
		if (auto err = sym.add_float_parameter_semantic(i, name); err.has_value()) {
			return err.value();
		}
	}

	return sym;
}

bool ShaderPassCompiler::validate_resources(spirv_cross::ShaderResources &p_vert, spirv_cross::ShaderResources &p_frag) {
	return p_vert.sampled_images.empty() &&
			p_vert.storage_buffers.empty() &&
			p_vert.subpass_inputs.empty() &&
			p_vert.storage_images.empty() &&
			p_vert.atomic_counters.empty() &&
			p_frag.storage_buffers.empty() &&
			p_frag.subpass_inputs.empty() &&
			p_frag.storage_images.empty() &&
			p_frag.atomic_counters.empty();
}

result::Result<ShaderPassReflection> ShaderPassCompiler::reflect(int p_pass_number, ShaderSymbols &p_sym, Compiler p_vert, Compiler p_frag) {
	ShaderPassReflection ref(p_pass_number);
	auto vs_res = p_vert->get_shader_resources();
	auto fs_res = p_frag->get_shader_resources();

	// validate resources
	if (!validate_resources(vs_res, fs_res)) {
		return error::Error::failed("invalid resources");
	}

	// validate input to vertex shader
	if (vs_res.stage_inputs.size() != 2) {
		return error::Error::failed("vertex shader input must have two attributes");
	}

	uint mask = 0;
	mask |= 1 << p_vert->get_decoration(vs_res.stage_inputs[0].id, spv::DecorationLocation);
	mask |= 1 << p_vert->get_decoration(vs_res.stage_inputs[1].id, spv::DecorationLocation);
	if (mask != 3) {
		return error::Error::failed("vertex shader input attributes must use (location = 0) and (location = 1)");
	}

	// validate number of render targets for fragment shader
	if (fs_res.stage_outputs.size() != 1) {
		return error::Error::failed("vertex shader must have a single output");
	}

	auto fs_output = fs_res.stage_outputs[0];
	if (p_frag->get_decoration(fs_output.id, spv::DecorationLocation) != 0) {
		return error::Error::failed("fragment shader output must use (location = 0)");
	}

	using Resource = spirv_cross::Resource;
	using Resources = spirv_cross::SmallVector<Resource>;
	auto one_or_none = [](Resources &resources) -> result::Result<Resource *> {
		if (resources.size() > 1) {
			return error::Error::failed("shader must use zero or one uniform buffer");
		}
		return resources.empty() ? nullptr : resources.begin();
	};

	// vertex UBO binding
	if (auto result = one_or_none(vs_res.uniform_buffers); result.is_ok() && result.ok() != nullptr) {
		auto &res = *result.take();
		if (p_vert->get_decoration(res.id, spv::DecorationDescriptorSet) != 0) {
			return error::Error::failed("vertex shader resources must use descriptor set #0");
		}
		auto &desc = ref.ubo;
		desc.binding = p_vert->get_decoration(res.id, spv::DecorationBinding);
		desc.stage = STAGE_VERTEX;
		desc.size = std::max(desc.size, (int)p_vert->get_declared_struct_size(p_vert->get_type(res.base_type_id)));

		if (auto err = add_active_buffer_ranges(ref, p_sym, p_vert, res, true); err.has_value()) {
			return err.value();
		}
	} else if (result.is_err()) {
		return result.take_err();
	}

	// fragment UBO binding
	if (auto result = one_or_none(fs_res.uniform_buffers); result.is_ok() && result.ok() != nullptr) {
		auto &res = *result.take();
		if (p_frag->get_decoration(res.id, spv::DecorationDescriptorSet) != 0) {
			return error::Error::failed("fragment shader resources must use descriptor set #0");
		}
		auto &desc = ref.ubo;
		auto bind = p_frag->get_decoration(res.id, spv::DecorationBinding);
		if (desc.binding != -1u && desc.binding != bind) {
			return error::Error::failed("vertex and fragment shader UBO bindings must match");
		}
		desc.binding = p_frag->get_decoration(res.id, spv::DecorationBinding);
		desc.stage |= Stage::STAGE_FRAGMENT;
		desc.size = std::max(desc.size, (int)p_frag->get_declared_struct_size(p_frag->get_type(res.base_type_id)));

		if (auto err = add_active_buffer_ranges(ref, p_sym, p_frag, res, true); err.has_value()) {
			return err.value();
		}
	} else if (result.is_err()) {
		return result.take_err();
	}

	// vertex Push binding
	if (auto result = one_or_none(vs_res.push_constant_buffers); result.is_ok() && result.ok() != nullptr) {
		auto &res = *result.take();
		auto dset = p_vert->get_decoration(res.id, spv::DecorationDescriptorSet);
		ref.push.size = std::max(ref.push.size, (int)p_vert->get_declared_struct_size(p_vert->get_type(res.base_type_id)));
		ref.push.stage = STAGE_VERTEX;

		if (auto err = add_active_buffer_ranges(ref, p_sym, p_vert, res, false); err.has_value()) {
			return err.value();
		}
	} else if (result.is_err()) {
		return result.take_err();
	}

	// fragment Push binding
	if (auto result = one_or_none(fs_res.push_constant_buffers); result.is_ok() && result.ok() != nullptr) {
		auto &res = *result.take();
		auto dset = p_frag->get_decoration(res.id, spv::DecorationDescriptorSet);
		ref.push.size = std::max(ref.push.size, (int)p_frag->get_declared_struct_size(p_frag->get_type(res.base_type_id)));
		ref.push.stage |= STAGE_FRAGMENT;

		if (auto err = add_active_buffer_ranges(ref, p_sym, p_frag, res, false); err.has_value()) {
			return err.value();
		}
	} else if (result.is_err()) {
		return result.take_err();
	}

	{
		uint32_t bindings = 0;
		for (auto &res : fs_res.sampled_images) {
			if (p_frag->get_decoration(res.id, spv::DecorationDescriptorSet) != 0) {
				return error::Error::failed("fragment shader texture must use descriptor set #0");
			}

			auto binding = p_frag->get_decoration(res.id, spv::DecorationBinding);
			if (binding == -1) {
				continue;
			}

			if (binding > (int)Limits::MAX_TEXTURE_BINDINGS) {
				std::ostringstream oss;
				oss << "fragment shader texture binding exceeds " << (int)Limits::MAX_TEXTURE_BINDINGS;
				return error::Error::failed(oss.str());
			}

			if ((bindings & (1 << binding)) != 0) {
				std::ostringstream oss;
				oss << "fragment shader texture binding " << binding << " already in use";
				return error::Error::failed(oss.str());
			}

			bindings |= 1 << binding;
			auto name = p_frag->get_name(res.id);
			auto sem_res = p_sym.get_texture_semantic_for_name(name);
			if (!sem_res.has_value()) {
				std::ostringstream oss;
				oss << "invalid texture " << name;
				return error::Error::failed(oss.str());
			}
			auto &sem = sem_res.value();
			ref.set_binding(binding, sem->semantic, sem->index, sem->name);
		}
	}

	ref.to_stream(std::cout);

	return ref;
}

error::ErrorOpt ShaderPassCompiler::add_active_buffer_ranges(ShaderPassReflection &p_ref, ShaderSymbols &p_sym, Compiler p_comp, spirv_cross::Resource &p_res, bool p_ubo) {
	auto ranges = p_comp->get_active_buffer_ranges(p_res.id);
	for (auto &range : ranges) {
		auto &name = p_comp->get_member_name(p_res.base_type_id, range.index);
		auto &type = p_comp->get_type(p_comp->get_type(p_res.base_type_id).member_types[range.index]);

		if (auto res = p_sym.get_buffer_semantic_for_uniform(name); res.has_value()) {
			auto &sem = res.value();
			if (!sem->valid_type(type)) {
				return error::Error::failed("invalid type");
			}

			auto vec_sz = type.vecsize;
			auto cols = type.columns;

			if (sem->semantic == SBS::FLOAT_PARAMETER) {
				if (auto err = p_ref.set_offset(range.offset, vec_sz, sem->index, name, p_ubo); err.has_value()) {
					return err;
				}
			} else {
				if (auto err = p_ref.set_offset(range.offset, vec_sz * cols, sem->semantic, p_ubo); err.has_value()) {
					return err;
				}
			}
		} else if (auto opt = p_sym.get_texture_semantic_for_uniform_name(name); opt.has_value()) {
			auto &tex_sem = opt.value();
			if (tex_sem->semantic == SBS::PASS_OUTPUT_SIZE && tex_sem->index >= p_ref.pass_number) {
				std::ostringstream oss;
				oss << "shader pass" << p_ref.pass_number << " is attempting to use texture semantic "
					<< tex_sem->name << " from pass " << tex_sem->index
					<< " which is the same or a later pass";
				return error::Error::failed(oss.str());
			}

			if (!tex_sem->valid_type(type)) {
				std::ostringstream oss;
				oss << "invalid type for texture semantic "
					<< tex_sem->name << " expected a vec4 of type float";
				return error::Error::failed(oss.str());
			}

			if (auto err = p_ref.set_offset(range.offset, tex_sem->semantic, tex_sem->index, tex_sem->name, p_ubo); err.has_value()) {
				return err;
			}
		}
	}

	return {};
}

compiled::UBOBufferDescriptor ShaderPassCompiler::make_ubo_descriptor(slang::ShaderPassReflection &p_ref) const {
	auto desc = make_descriptors(p_ref, ShaderBufferSemanticMeta::UBO);

	return compiled::UBOBufferDescriptor(p_ref.ubo.binding, (compiled::Stage)p_ref.ubo.stage, p_ref.ubo.size, desc);
}

compiled::PushBufferDescriptor ShaderPassCompiler::make_push_descriptor(slang::ShaderPassReflection &p_ref) const {
	auto desc = make_descriptors(p_ref, ShaderBufferSemanticMeta::PUSH);

	return compiled::PushBufferDescriptor((compiled::Stage)p_ref.push.stage, p_ref.push.size, desc);
}

std::vector<compiled::BufferUniformDescriptor> ShaderPassCompiler::make_descriptors(slang::ShaderPassReflection &p_ref, ShaderBufferSemanticMeta::OffsetType p_type) const {
	// Find bound global semantics, like MVP, FrameCount, etc
	std::vector<compiled::BufferUniformDescriptor> descriptors;
	for (auto &kv : p_ref.semantics) {
		compiled::ShaderBufferSemantic sem = kv.first;
		ShaderBufferSemanticMetaRef meta = kv.second;
		if (auto offset = meta->get_offset(p_type); offset.has_value()) {
			descriptors.push_back(
					compiled::BufferUniformDescriptor(sem,
							std::nullopt,
							meta->name,
							meta->number_of_components * sizeof(float),
							offset.value()));
		}
	}

	// Find bound parameters
	for (auto &kv : p_ref.float_parameters) {
		ShaderBufferSemanticMetaRef meta = kv.second;
		if (auto offset = meta->get_offset(p_type); offset.has_value()) {
			descriptors.push_back(
					compiled::BufferUniformDescriptor(SBS::FLOAT_PARAMETER,
							meta->index,
							meta->name,
							meta->number_of_components * sizeof(float),
							offset.value()));
		}
	}

	// Find bound texture sizes such as OriginalSize, <LUT alias>Size, etc
	for (auto &kv : p_ref.texture_uniforms) {
		compiled::ShaderBufferSemantic sem = kv.first;
		auto &a = kv.second;
		for (auto &kv2 : a) {
			ShaderBufferSemanticMetaRef meta = kv2.second;
			if (auto offset = meta->get_offset(p_type); offset.has_value()) {
				descriptors.push_back(
						compiled::BufferUniformDescriptor(sem,
								meta->index,
								meta->name,
								4 * sizeof(float), // these are always vec4
								offset.value()));
			}
		}
	}

	return descriptors;
}

std::vector<compiled::TextureDescriptor> ShaderPassCompiler::make_textures(ShaderPassReflection &p_ref, ShaderSymbols &p_sym) const {
	std::vector<compiled::TextureDescriptor> textures;

	for (auto &kv : p_ref.textures) {
		compiled::ShaderTextureSemantic sem = kv.first;
		auto &a = kv.second;
		for (auto &kv2 : a) {
			auto &meta = kv2.second;
			if (meta->binding.has_value()) {
				auto binding = meta->binding.value();
				auto wrap = sem == STS::USER ? shader->luts[meta->index].wrap_mode : shader->passes[p_ref.pass_number].wrap_mode;
				auto filter = sem == STS::USER ? shader->luts[meta->index].filter : shader->passes[p_ref.pass_number].filter;
				textures.push_back(
						compiled::TextureDescriptor(meta->name,
								sem,
								binding,
								wrap,
								filter,
								meta->index));
			}
		}
	}

	return textures;
}

static_assert(Stage::STAGE_NONE == compiled::Stage::STAGE_NONE);
static_assert(Stage::STAGE_VERTEX == compiled::Stage::STAGE_VERTEX);
static_assert(Stage::STAGE_FRAGMENT == compiled::Stage::STAGE_FRAGMENT);

} // namespace slang
