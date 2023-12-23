//
// Created by Stuart Carnie on 26/10/2023.
//

#ifndef GODOT_TYPES_H
#define GODOT_TYPES_H

#include "../../../thirdparty/spirv-cross/spirv.hpp"
#import "core/io/marshalls.h"
#import <Metal/Metal.h>

int decompress(uint8_t *p_dst, int p_dst_max_size, const uint8_t *p_src, int p_src_size);

namespace RD {
enum UniformType {
	UNIFORM_TYPE_SAMPLER, //for sampling only (sampler GLSL type)
	UNIFORM_TYPE_SAMPLER_WITH_TEXTURE, // for sampling only, but includes a texture, (samplerXX GLSL type), first a sampler then a texture
	UNIFORM_TYPE_TEXTURE, //only texture, (textureXX GLSL type)
	UNIFORM_TYPE_IMAGE, // storage image (imageXX GLSL type), for compute mostly
	UNIFORM_TYPE_TEXTURE_BUFFER, // buffer texture (or TBO, textureBuffer type)
	UNIFORM_TYPE_SAMPLER_WITH_TEXTURE_BUFFER, // buffer texture with a sampler(or TBO, samplerBuffer type)
	UNIFORM_TYPE_IMAGE_BUFFER, //texel buffer, (imageBuffer type), for compute mostly
	UNIFORM_TYPE_UNIFORM_BUFFER, //regular uniform buffer (or UBO).
	UNIFORM_TYPE_STORAGE_BUFFER, //storage buffer ("buffer" qualifier) like UBO, but supports storage, for compute mostly
	UNIFORM_TYPE_INPUT_ATTACHMENT, //used for sub-pass read/write, for mobile mostly
	UNIFORM_TYPE_MAX
};

enum PipelineSpecializationConstantType {
	PIPELINE_SPECIALIZATION_CONSTANT_TYPE_BOOL,
	PIPELINE_SPECIALIZATION_CONSTANT_TYPE_INT,
	PIPELINE_SPECIALIZATION_CONSTANT_TYPE_FLOAT,
};
} //namespace RD

namespace RD {
enum ShaderStage {
	SHADER_STAGE_VERTEX,
	SHADER_STAGE_FRAGMENT,
	SHADER_STAGE_TESSELATION_CONTROL,
	SHADER_STAGE_TESSELATION_EVALUATION,
	SHADER_STAGE_COMPUTE,
	SHADER_STAGE_MAX,
	SHADER_STAGE_VERTEX_BIT = (1 << SHADER_STAGE_VERTEX),
	SHADER_STAGE_FRAGMENT_BIT = (1 << SHADER_STAGE_FRAGMENT),
	SHADER_STAGE_TESSELATION_CONTROL_BIT = (1 << SHADER_STAGE_TESSELATION_CONTROL),
	SHADER_STAGE_TESSELATION_EVALUATION_BIT = (1 << SHADER_STAGE_TESSELATION_EVALUATION),
	SHADER_STAGE_COMPUTE_BIT = (1 << SHADER_STAGE_COMPUTE),
};

}

enum ShaderStageUsage : uint8_t {
	None = 0,
	Vertex = RD::SHADER_STAGE_VERTEX_BIT,
	Fragment = RD::SHADER_STAGE_FRAGMENT_BIT,
	TesselationControl = RD::SHADER_STAGE_TESSELATION_CONTROL_BIT,
	TesselationEvaluation = RD::SHADER_STAGE_TESSELATION_EVALUATION_BIT,
	Compute = RD::SHADER_STAGE_COMPUTE_BIT,
};

inline ShaderStageUsage &operator|=(ShaderStageUsage &a, int b) {
	a = ShaderStageUsage(uint8_t(a) | uint8_t(b));
	return a;
}

struct BindingInfo {
	MTLDataType dataType = MTLDataTypeNone;
	uint32_t index = 0;
	MTLBindingAccess access = MTLBindingAccessReadOnly;
	MTLResourceUsage usage = 0;
	MTLTextureType textureType = MTLTextureType2D;
	spv::ImageFormat imageFormat = spv::ImageFormatUnknown;
	uint32_t arrayLength = 0;
	bool isMultisampled = false;
};

class BufReader;

template <typename T>
concept Deserializable = requires(T t, BufReader &p_reader) {
	{ t.deserialize(p_reader) } -> std::same_as<void>;
};

class BufReader {
	uint8_t const *data = nullptr;
	uint64_t length = 0;
	uint64_t pos = 0;

	bool check_length(size_t p_size) {
		if (status != Status::OK)
			return false;

		if (pos + p_size > length) {
			status = Status::SHORT_BUFFER;
			return false;
		}
		return true;
	}

#define CHECK(p_size)          \
	if (!check_length(p_size)) \
	return

public:
	enum class Status {
		OK,
		SHORT_BUFFER,
		BAD_COMPRESSION,
	};

	Status status = Status::OK;

	BufReader(uint8_t const *p_data, uint64_t p_length) :
			data(p_data), length(p_length) {}

	template <Deserializable T>
	void read(T &p_value) {
		p_value.deserialize(*this);
	}

	void skip(size_t p_size) {
		CHECK(p_size);
		pos += p_size;
	}

	_FORCE_INLINE_ void read(uint32_t &p_val) {
		CHECK(sizeof(uint32_t));

		p_val = decode_uint32(data + pos);
		pos += sizeof(uint32_t);
	}

	_FORCE_INLINE_ void read(bool &p_val) {
		CHECK(sizeof(uint8_t));

		p_val = *(data + pos) > 0;
		pos += 1;
	}

	_FORCE_INLINE_ void read(uint64_t &p_val) {
		CHECK(sizeof(uint64_t));

		p_val = decode_uint64(data + pos);
		pos += sizeof(uint64_t);
	}

	_FORCE_INLINE_ void read(float &p_val) {
		CHECK(sizeof(float));

		p_val = decode_float(data + pos);
		pos += sizeof(float);
	}

	_FORCE_INLINE_ void read(double &p_val) {
		CHECK(sizeof(double));

		p_val = decode_double(data + pos);
		pos += sizeof(double);
	}

	void read(std::shared_ptr<char> &p_val) {
		uint32_t len;
		read(len);
		CHECK(len);
		p_val.reset((char *)malloc(len + 1) /* NUL */);
		memcpy(p_val.get(), data + pos, len);
		p_val.get()[len] = 0;
		pos += len;
	}

	void read_compressed(std::shared_ptr<char> &p_val) {
		uint32_t len;
		read(len);
		uint32_t comp_size;
		read(comp_size);

		CHECK(comp_size);

		p_val.reset((char *)malloc(len + 1));
		int bytes = decompress(reinterpret_cast<uint8_t *>(p_val.get()), len, data + pos, comp_size);
		if (bytes != len) {
			status = Status::BAD_COMPRESSION;
			return;
		}
		p_val.get()[len] = 0;
		pos += comp_size;
	}

	void read(std::vector<uint8_t> &p_val) {
		uint32_t len;
		read(len);
		CHECK(len);
		p_val.resize(len);
		memcpy(p_val.data(), data + pos, len);
		pos += len;
	}

	template <typename T>
	void read(std::vector<T> &p_val) {
		uint32_t len;
		read(len);
		CHECK(len);
		p_val.resize(len);
		for (int i = 0; i < len; i++) {
			read(p_val[i]);
		}
	}

	template <typename K, typename V>
	void read(std::map<K, V> &p_map) {
		uint32_t len;
		read(len);
		CHECK(len);
		for (uint32_t i = 0; i < len; i++) {
			K key;
			read(key);
			V value;
			read(value);
			p_map[key] = value;
		}
	}

#undef CHECK
};

const uint32_t R32UI_ALIGNMENT_CONSTANT_ID = 65535;

struct ComputeSize {
	uint32_t x;
	uint32_t y;
	uint32_t z;

	void deserialize(BufReader &p_reader) {
		p_reader.read(x);
		p_reader.read(y);
		p_reader.read(z);
	}
};

struct ShaderStageData {
	RD::ShaderStage stage;
	std::shared_ptr<char> entry_point_name;
	std::shared_ptr<char> source;

	void deserialize(BufReader &p_reader) {
		p_reader.read((uint32_t &)stage);
		p_reader.read(entry_point_name);
		p_reader.read_compressed(source);
	}
};

struct SpecializationConstantData {
	uint32_t constant_id;
	RD::PipelineSpecializationConstantType type;
	ShaderStageUsage stages;
	// used_stages specifies the stages the constant is used by Metal
	ShaderStageUsage used_stages;
	uint32_t int_value;

	void deserialize(BufReader &p_reader) {
		p_reader.read(constant_id);
		p_reader.read((uint32_t &)type);
		p_reader.read((uint32_t &)stages);
		p_reader.read((uint32_t &)used_stages);
		p_reader.read(int_value);
	}
};

struct UniformData {
	RD::UniformType type;
	uint32_t binding;
	bool writable;
	uint32_t length;
	ShaderStageUsage stages;
	// active_stages specifies the stages the uniform data is
	// used by the Metal shader
	ShaderStageUsage active_stages;
	std::map<RD::ShaderStage, BindingInfo> bindings;
	std::map<RD::ShaderStage, BindingInfo> bindings_secondary;

	void deserialize(BufReader &p_reader) {
		p_reader.read((uint32_t &)type);
		p_reader.read(binding);
		p_reader.read(writable);
		p_reader.read(length);
		p_reader.read((uint32_t &)stages);
		p_reader.read((uint32_t &)active_stages);
		uint32_t bindings_size;
		p_reader.read(bindings_size);
		for (uint32_t i = 0; i < bindings_size; i++) {
			RD::ShaderStage stage;
			BindingInfo info;
			p_reader.read((uint32_t &)stage);
			p_reader.read((uint32_t &)info.dataType);
			p_reader.read(info.index);
			p_reader.read((uint32_t &)info.access);
			p_reader.read((uint32_t &)info.usage);
			p_reader.read((uint32_t &)info.textureType);
			p_reader.read((uint32_t &)info.imageFormat);
			p_reader.read(info.arrayLength);
			p_reader.read(info.isMultisampled);
			bindings[stage] = info;
		}
		uint32_t bindings_secondary_size;
		p_reader.read(bindings_secondary_size);
		for (uint32_t i = 0; i < bindings_secondary_size; i++) {
			RD::ShaderStage stage;
			BindingInfo info;
			p_reader.read((uint32_t &)stage);
			p_reader.read((uint32_t &)info.dataType);
			p_reader.read(info.index);
			p_reader.read((uint32_t &)info.access);
			p_reader.read((uint32_t &)info.usage);
			p_reader.read((uint32_t &)info.textureType);
			p_reader.read((uint32_t &)info.imageFormat);
			p_reader.read(info.arrayLength);
			p_reader.read(info.isMultisampled);
			bindings_secondary[stage] = info;
		}
	}
};

struct UniformSetData {
	uint32_t index;
	std::vector<UniformData> uniforms;

	void deserialize(BufReader &p_reader) {
		p_reader.read(index);
		p_reader.read(uniforms);
	}
};

struct PushConstantData {
	uint32_t size;
	ShaderStageUsage stages;
	ShaderStageUsage used_stages;
	std::map<RD::ShaderStage, uint32_t> msl_binding;

	void deserialize(BufReader &p_reader) {
		p_reader.read(size);
		p_reader.read((uint32_t &)stages);
		p_reader.read((uint32_t &)used_stages);
		uint32_t msl_binding_size;
		p_reader.read(msl_binding_size);
		for (uint32_t i = 0; i < msl_binding_size; i++) {
			RD::ShaderStage stage;
			uint32_t binding;
			p_reader.read((uint32_t &)stage);
			p_reader.read(binding);
			msl_binding[stage] = binding;
		}
	}
};

struct ShaderBinaryData {
	std::shared_ptr<char> shader_name;
	uint32_t vertex_input_mask;
	uint32_t fragment_output_mask;
	uint32_t spirv_specialization_constants_ids_mask;
	uint32_t is_compute;
	ComputeSize compute_local_size;
	PushConstantData push_constant;
	std::vector<ShaderStageData> stages;
	std::vector<SpecializationConstantData> constants;
	std::vector<UniformSetData> uniforms;

	void deserialize(BufReader &p_reader) {
		p_reader.read(shader_name);
		p_reader.read(vertex_input_mask);
		p_reader.read(fragment_output_mask);
		p_reader.read(spirv_specialization_constants_ids_mask);
		p_reader.read(is_compute);
		p_reader.read(compute_local_size);
		p_reader.read(push_constant);
		p_reader.read(stages);
		p_reader.read(constants);
		p_reader.read(uniforms);
	}
};

static const char *shader_file_header = "GDSC";
static const uint32_t cache_file_version = 3;

#endif //GODOT_TYPES_H
