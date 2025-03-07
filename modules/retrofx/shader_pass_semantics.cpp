//
// Created by Stuart Carnie on 29/12/2024.
//

#include "shader_pass_semantics.h"

namespace shader::pass {

compiled::ShaderBufferSemantic to_buffer_semantic(const compiled::ShaderTextureSemantic p_sem) {
	switch (p_sem) {
		case compiled::ShaderTextureSemantic::ORIGINAL:
			return compiled::ShaderBufferSemantic::ORIGINAL_SIZE;
		case compiled::ShaderTextureSemantic::SOURCE:
			return compiled::ShaderBufferSemantic::SOURCE_SIZE;
		case compiled::ShaderTextureSemantic::ORIGINAL_HISTORY:
			return compiled::ShaderBufferSemantic::ORIGINAL_HISTORY_SIZE;
		case compiled::ShaderTextureSemantic::PASS_OUTPUT:
			return compiled::ShaderBufferSemantic::PASS_OUTPUT_SIZE;
		case compiled::ShaderTextureSemantic::PASS_FEEDBACK:
			return compiled::ShaderBufferSemantic::PASS_FEEDBACK_SIZE;
		case compiled::ShaderTextureSemantic::USER:
			return compiled::ShaderBufferSemantic::USER_SIZE;
	}
}

}
