//
// Created by Stuart Carnie on 4/1/2025.
//

#include "shader_pass_bindings.h"

namespace shader {
namespace pass {

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

void UBOBufferBinding::update(RD *p_rd) {
	if (binding.is_empty()) {
		return;
	}

	binding.update_data();
	p_rd->buffer_update(ubo_buffer, 0, binding.data.size(), binding.data.ptr());
}

void PushBufferBinding::update(RD *p_rd) {
	if (binding.is_empty()) {
		return;
	}

	binding.update_data();
}

} // namespace pass
} //namespace shader
