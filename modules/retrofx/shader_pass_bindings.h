//
// Created by Stuart Carnie on 28/12/2024.
//

#ifndef SHADER_PASS_BINDINGS_H
#define SHADER_PASS_BINDINGS_H

#include "servers/rendering/rendering_device.h"

#include <compiled.h>

namespace shader {
namespace pass {

struct UniformBinding {
	void *data = nullptr;
	size_t size = 0;
	size_t offset = 0;
	String name;
};

struct BufferBinding {
	Vector<uint8_t> data;
	Vector<UniformBinding> uniforms;

	void add_uniform(void *p_data, size_t p_size, size_t p_offset, const String &p_name) {
		uniforms.push_back({ .data = p_data, .size = p_size, .offset = p_offset, .name = p_name });
	}

	bool is_empty() const {
		return uniforms.is_empty();
	}

	void update_data();
};

struct UBOBufferBinding {
	RID ubo_buffer;
	BufferBinding binding;

	void update(RD *p_rd);
};

struct PushBufferBinding {
	BufferBinding binding;

	void update(RD *p_rd);
};

struct TextureBinding {
	RD::Uniform *uniform = nullptr;
	RID *texture;
	String name;
};

struct Bindings {
	UBOBufferBinding ubo;
	PushBufferBinding push;
	LocalVector<RD::Uniform> uniforms;
	Vector<TextureBinding> textures;

	/// <code>true</code> if the pass uses feedback or history, which requires texture updates
	bool needs_texture_update = false;

	void add_texture(RD::Uniform *p_uniform, RID *p_texture, const String &p_name) {
		textures.push_back({ .uniform = p_uniform, .texture = p_texture, .name = p_name });
	}

	void sort() {
		// TODO: sort by binding
	}

	void free(RD *p_rd) {
		if (ubo.ubo_buffer.is_valid()) {
			p_rd->free(ubo.ubo_buffer);
		}
	};
};

} //namespace pass
} //namespace shader

#endif //SHADER_PASS_BINDINGS_H
