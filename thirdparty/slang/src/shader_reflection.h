//
// Created by Stuart Carnie on 12/8/2024.
//

#pragma once

#include "compiled.h"

#include <spirv_cross.hpp>

#include <optional>

namespace slang {

using std::optional;

struct ShaderTextureSemanticMeta {
	int index;
	std::string name;
	optional<int> binding;

	ShaderTextureSemanticMeta(int p_index, std::string name) :
			index(p_index), name(std::move(name)) {}

	static std::shared_ptr<ShaderTextureSemanticMeta> create(int p_index, std::string name) {
		return std::make_shared<ShaderTextureSemanticMeta>(p_index, std::move(name));
	}
};

using ShaderTextureSemanticMetaRef = std::shared_ptr<ShaderTextureSemanticMeta>;

struct ShaderBufferSemanticMeta {
	int index = 0;
	std::string name;
	optional<int> ubo_offset;
	optional<int> push_offset;
	int number_of_components = 0;

	enum OffsetType {
		UBO = 0,
		PUSH = 1
	};

	optional<int> get_offset(OffsetType p_type) const {
		return p_type == UBO ? ubo_offset : push_offset;
	}

	ShaderBufferSemanticMeta(ShaderBufferSemanticMeta &&p_other) = default;

	ShaderBufferSemanticMeta(const std::string &p_name) :
			ShaderBufferSemanticMeta(0, p_name) {
	}

	ShaderBufferSemanticMeta(compiled::ShaderBufferSemantic p_sem) :
			ShaderBufferSemanticMeta(0, compiled::to_string(p_sem)) {
	}

	static std::shared_ptr<ShaderBufferSemanticMeta> create(compiled::ShaderBufferSemantic p_sem) {
		return std::make_shared<ShaderBufferSemanticMeta>(p_sem);
	}

	ShaderBufferSemanticMeta(int p_index, std::string p_name) :
			index(p_index), name(std::move(p_name)) {
	}

	static std::shared_ptr<ShaderBufferSemanticMeta> create(int p_index, std::string p_name) {
		return std::make_shared<ShaderBufferSemanticMeta>(p_index, std::move(p_name));
	}
};

using ShaderBufferSemanticMetaRef = std::shared_ptr<ShaderBufferSemanticMeta>;

struct ShaderTextureSemanticMap {
	compiled::ShaderTextureSemantic semantic = compiled::ShaderTextureSemantic::ORIGINAL;
	int index = 0;
	std::string name;

	ShaderTextureSemanticMap(ShaderTextureSemanticMap &&p_other) = default;

	ShaderTextureSemanticMap(compiled::ShaderTextureSemantic p_semantic, int p_index, const std::string &p_name) :
			semantic(p_semantic), index(p_index), name(p_name) {}

	static std::shared_ptr<ShaderTextureSemanticMap> create(compiled::ShaderTextureSemantic p_semantic, int p_index, const std::string &p_name) {
		return std::make_shared<ShaderTextureSemanticMap>(p_semantic, p_index, p_name);
	}
};

using ShaderTextureSemanticMapRef = std::shared_ptr<ShaderTextureSemanticMap>;

struct ShaderBufferSemanticMap {
	compiled::ShaderBufferSemantic semantic;
	uint32_t index;
	std::string name;
	spirv_cross::SPIRType::BaseType base_type;
	uint32_t vec_size;
	uint32_t cols;

	ShaderBufferSemanticMap(ShaderBufferSemanticMap &&p_other) = default;

	ShaderBufferSemanticMap(compiled::ShaderBufferSemantic p_semantic,
			uint32_t p_index,
			const std::string &p_name,
			spirv_cross::SPIRType::BaseType p_base_type,
			uint32_t p_vec_size,
			uint32_t p_cols) :
			semantic(p_semantic), index(p_index), name(p_name), base_type(p_base_type), vec_size(p_vec_size), cols(p_cols) {}

	static std::shared_ptr<ShaderBufferSemanticMap> create(compiled::ShaderBufferSemantic p_semantic,
			uint32_t p_index,
			const std::string &p_name,
			spirv_cross::SPIRType::BaseType p_base_type,
			uint32_t p_vec_size,
			uint32_t p_cols) {
		return std::make_shared<ShaderBufferSemanticMap>(p_semantic, p_index, p_name, p_base_type, p_vec_size, p_cols);
	}

	ShaderBufferSemanticMap(compiled::ShaderBufferSemantic p_semantic,
			spirv_cross::SPIRType::BaseType p_base_type,
			uint32_t p_vec_size,
			uint32_t p_cols) :
			semantic(p_semantic),
			index(0),
			name(compiled::to_string(p_semantic)),
			base_type(p_base_type),
			vec_size(p_vec_size),
			cols(p_cols) {}

	static std::shared_ptr<ShaderBufferSemanticMap> create(compiled::ShaderBufferSemantic p_semantic, spirv_cross::SPIRType::BaseType p_base_type, int p_vec_size, int p_cols) {
		return std::make_shared<ShaderBufferSemanticMap>(p_semantic, p_base_type, p_vec_size, p_cols);
	}

	bool valid_type(spirv_cross::SPIRType const &p_type) const {
		return p_type.array.size() == 0 &&
				p_type.basetype == base_type &&
				p_type.vecsize == vec_size &&
				p_type.columns == cols;
	}
};

using ShaderBufferSemanticMapRef = std::shared_ptr<ShaderBufferSemanticMap>;

enum Stage : uint8_t {
	STAGE_NONE = 0,
	STAGE_VERTEX = 1 << 0,
	STAGE_FRAGMENT = 1 << 1,
};

inline Stage operator|(Stage a, Stage b) {
	return static_cast<Stage>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline Stage &operator|=(Stage &a, Stage b) {
	a = a | b;
	return a;
}

struct UBOBindingDescriptor {
	int size = 0;
	uint32_t binding = -1u;
	Stage stage = STAGE_NONE;

	bool is_valid() const { return size > 0 && binding != -1u; }
	bool is_vert() const { return is_valid() && (stage & STAGE_VERTEX); }
	bool is_frag() const { return is_valid() && (stage & STAGE_FRAGMENT); }
};

struct PushBindingDescriptor {
	int size = 0;
	Stage stage = STAGE_NONE;

	bool is_valid() const { return size > 0; }
	bool is_vert() const { return is_valid() && (stage & STAGE_VERTEX); }
	bool is_frag() const { return is_valid() && (stage & STAGE_FRAGMENT); }
};

} //namespace slang
