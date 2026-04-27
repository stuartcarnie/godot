//
// Created by Stuart Carnie on 4/1/2025.
//

#include "shader_pass_bindings.h"

namespace shader::pass {

void BufferBinding::update_data() {
	if (data.size() == 0) {
		return;
	}

	uintptr_t data_ptr = (uintptr_t)data.ptrw();

	for (auto &u : uniforms) {
		void *dst = (void *)(data_ptr + u.offset);
		memcpy(dst, u.data, u.size);
	}
}

void BufferBinding::reset(RD *p_rd) {
	data.clear();
	uniforms.clear();
}

void UBOBufferBinding::update(RD *p_rd) {
	if (binding.is_empty()) {
		return;
	}

	binding.update_data();
	p_rd->buffer_update(ubo_buffer, 0, binding.data.size(), binding.data.ptr());
}

void UBOBufferBinding::reset(RD *p_rd) {
	binding.reset(p_rd);

	if (ubo_buffer.is_valid()) {
		p_rd->free_rid(ubo_buffer);
		ubo_buffer = RID();
	}
}

void PushBufferBinding::update(RD *p_rd) {
	if (binding.is_empty()) {
		return;
	}

	binding.update_data();
}

void PushBufferBinding::reset(RD *p_rd) {
	size = 0;
	binding.reset(p_rd);
}

} //namespace shader::pass
