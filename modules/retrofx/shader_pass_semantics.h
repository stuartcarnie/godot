//
// Created by Stuart Carnie on 28/12/2024.
//

#pragma once

#include "core/templates/hash_map.h"
#include "core/templates/rid.h"

#include <compiled.h>

namespace shader::pass {

using namespace slang;

compiled::ShaderBufferSemantic to_buffer_semantic(const compiled::ShaderTextureSemantic p_sem);

struct BufferSemantics {
	void *data = nullptr;
};

struct TextureSemantics {
	RID *texture;
	size_t stride = 0;
};

struct TextureUniformSemantics {
	void *size = nullptr;
	size_t stride = 0;
};

struct Semantics {
	HashMap<compiled::ShaderTextureSemantic, TextureSemantics, compiled::Hashers> textures;
	HashMap<compiled::ShaderBufferSemantic, TextureUniformSemantics, compiled::Hashers> texture_uniforms;
	HashMap<compiled::ShaderBufferSemantic, BufferSemantics, compiled::Hashers> uniforms;
	HashMap<int, BufferSemantics> float_parameters;

	void add_texture(RID *p_texture, void *p_size, compiled::ShaderTextureSemantic p_semantic) {
		textures[p_semantic] = { .texture = p_texture, 0 };
		texture_uniforms[to_buffer_semantic(p_semantic)] = { .size = p_size };
	}

	void add_texture(RID *p_texture, size_t p_texture_stride,
			void *p_size, size_t p_size_stride, compiled::ShaderTextureSemantic p_semantic) {
		textures[p_semantic] = { .texture = p_texture, .stride = p_texture_stride };
		texture_uniforms[to_buffer_semantic(p_semantic)] = { .size = p_size, .stride = p_size_stride };
	}

	void add_uniform_data(void *p_data, compiled::ShaderBufferSemantic p_semantic) {
		uniforms[p_semantic] = { .data = p_data };
	}

	void add_float_parameter(void *p_data, int p_index) {
		float_parameters[p_index] = { .data = p_data };
	}

	optional<BufferSemantics> get_float_parameter(int p_index) {
		if (BufferSemantics *v = float_parameters.getptr(p_index)) {
			return *v;
		}
		return {};
	}
};

}
