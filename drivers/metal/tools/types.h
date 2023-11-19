//
// Created by Stuart Carnie on 26/10/2023.
//

#ifndef GODOT_TYPES_H
#define GODOT_TYPES_H

#import <Metal/Metal.h>
#include "../../../thirdparty/spirv-cross/spirv.hpp"
#include "../cista.h"

namespace data = cista::offset;

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

enum PipelineSpecializationConstantType {
	PIPELINE_SPECIALIZATION_CONSTANT_TYPE_BOOL,
	PIPELINE_SPECIALIZATION_CONSTANT_TYPE_INT,
	PIPELINE_SPECIALIZATION_CONSTANT_TYPE_FLOAT,
};
} //namespace RD

namespace RDM {
using namespace RD;
enum ShaderStageUsage : uint8_t {
	None = 0,
	Vertex = SHADER_STAGE_VERTEX_BIT,
	Fragment = SHADER_STAGE_FRAGMENT_BIT,
	TesselationControl = SHADER_STAGE_TESSELATION_CONTROL_BIT,
	TesselationEvaluation = SHADER_STAGE_TESSELATION_EVALUATION_BIT,
	Compute = SHADER_STAGE_COMPUTE_BIT,
};

inline ShaderStageUsage &operator|=(ShaderStageUsage &a, int b) {
	a = ShaderStageUsage(uint8_t(a) | uint8_t(b));
	return a;
}

enum LengthType {
	Bytes,
	Array
};

struct BindingInfo {
	MTLDataType dataType;
	uint32_t index;
	MTLBindingAccess access;
	MTLTextureType textureType = MTLTextureType2D;
	spv::ImageFormat imageFormat = spv::ImageFormatUnknown;
	uint32_t arrayLength;
	bool isMultisampled = false;

	inline auto newArgumentDescriptor() -> MTLArgumentDescriptor * {
		MTLArgumentDescriptor *desc = MTLArgumentDescriptor.argumentDescriptor;
		desc.dataType = dataType;
		desc.index = index;
		desc.access = access;
		desc.textureType = textureType;
		desc.arrayLength = arrayLength;
		return desc;
	}
};

} //namespace RDM

struct ComputeSize {
	uint32_t x;
	uint32_t y;
	uint32_t z;
};

struct ShaderStageData {
	RD::ShaderStage stage;
	data::string entry_point_name;
	data::vector<uint8_t> source_data;
	uint32_t source_size;
};

struct SpecializationConstantData {
	uint32_t constant_id;
	RD::PipelineSpecializationConstantType type;
	// used_stages specifies the stages the constant is used by Metal
	RDM::ShaderStageUsage used_stages;
	uint32_t int_value;
};

struct UniformData {
	RD::UniformType type;
	uint32_t binding;
	RDM::LengthType length_type;
	uint32_t length;
	RDM::ShaderStageUsage stages;
	// used_stages specifies the stages the uniform data is used by Metal
	RDM::ShaderStageUsage active_stages;
	data::hash_map<RD::ShaderStage, RDM::BindingInfo> bindings;
	data::hash_map<RD::ShaderStage, RDM::BindingInfo> bindings_secondary;
};

struct UniformSetData {
	uint32_t index;
	data::vector<UniformData> uniforms;
};

struct PushConstantData {
	uint32_t size;
	RDM::ShaderStageUsage stages;
	RDM::ShaderStageUsage used_stages;
	data::hash_map<RD::ShaderStage, uint32_t> msl_binding;
};

struct ShaderBinaryData {
	data::string shader_name;
	uint32_t vertex_input_mask;
	uint32_t fragment_output_mask;
	uint32_t spirv_specialization_constants_ids_mask;
	uint32_t is_compute;
	ComputeSize compute_local_size;
	PushConstantData push_constant;
	data::vector<ShaderStageData> stages;
	data::vector<SpecializationConstantData> constants;
	data::vector<UniformSetData> uniforms;
};

static const char *shader_file_header = "GDSC";
static const uint32_t cache_file_version = 3;

// Generic swap template.
#ifndef SWAP
#define SWAP(m_x, m_y) __swap_tmpl((m_x), (m_y))
template <class T>
inline void __swap_tmpl(T &x, T &y) {
	T aux = x;
	x = y;
	y = aux;
}
#endif // SWAP

class Reader {
private:
	std::vector<char> data;
	mutable size_t pos = 0;

	bool big_endian = false;

public:
	Reader(std::vector<char> &buf) :
			data(std::move(buf)) {}

	uint8_t get_8() const {
		uint8_t ret = 0;
		if (pos < data.size()) {
			ret = data[pos];
		}
		++pos;

		return ret;
	}

	uint16_t get_16() const {
		uint16_t res;
		uint8_t a, b;

		a = get_8();
		b = get_8();

		if (big_endian) {
			SWAP(a, b);
		}

		res = b;
		res <<= 8;
		res |= a;

		return res;
	}

	uint32_t get_32() const {
		uint32_t res;
		uint16_t a, b;

		a = get_16();
		b = get_16();

		if (big_endian) {
			SWAP(a, b);
		}

		res = b;
		res <<= 16;
		res |= a;

		return res;
	}

	uint64_t get_64() const {
		uint64_t res;
		uint32_t a, b;

		a = get_32();
		b = get_32();

		if (big_endian) {
			SWAP(a, b);
		}

		res = b;
		res <<= 32;
		res |= a;

		return res;
	}

	uint64_t get_buffer(uint8_t *p_dst, uint64_t p_length) {
		uint64_t left = data.size() - pos;
		uint64_t read = MIN(p_length, left);

		memcpy(p_dst, &data[pos], read);
		pos += read;

		return read;
	}
};

int decompress(uint8_t *p_dst, int p_dst_max_size, const uint8_t *p_src, int p_src_size);

#endif //GODOT_TYPES_H
