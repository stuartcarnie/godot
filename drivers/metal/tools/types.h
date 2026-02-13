//
// Binary layout definitions for Godot shader container format.
//
// Source: servers/rendering/rendering_shader_container.h
// Source: drivers/metal/rendering_shader_container_metal.h
//

#ifndef GODOT_TYPES_H
#define GODOT_TYPES_H

#include <cstddef>
#include <cstdint>

static inline size_t aligned_to(size_t p_size, size_t p_alignment) {
	size_t rem = p_size % p_alignment;
	return rem ? p_size + (p_alignment - rem) : p_size;
}

// Outer cache file format (servers/rendering/renderer_rd/shader_rd.cpp).
static const char *shader_file_header = "GDSC";
static const uint32_t cache_file_version = 4;

// Inner container (servers/rendering/rendering_shader_container.h).
static const uint32_t CONTAINER_MAGIC = 0x43535247;
static const uint32_t METAL_FORMAT = 0x42424242;

enum ShaderStage : uint32_t {
	SHADER_STAGE_VERTEX = 0,
	SHADER_STAGE_FRAGMENT = 1,
	SHADER_STAGE_TESSELATION_CONTROL = 2,
	SHADER_STAGE_TESSELATION_EVALUATION = 3,
	SHADER_STAGE_COMPUTE = 4,
	SHADER_STAGE_MAX,
};

enum CompressionFlags : uint32_t {
	COMPRESSION_FLAG_ZSTD = 0x1,
};

// RenderingShaderContainer::ContainerHeader
struct ContainerHeader {
	uint32_t magic_number;
	uint32_t version;
	uint32_t format;
	uint32_t format_version;
	uint32_t shader_count;
};

// RenderingShaderContainer::ReflectionData
struct ReflectionData {
	uint64_t vertex_input_mask;
	uint32_t fragment_output_mask;
	uint32_t specialization_constants_count;
	uint32_t pipeline_type;
	uint32_t has_multiview;
	uint32_t has_dynamic_buffers;
	uint32_t compute_local_size[3];
	uint32_t set_count;
	uint32_t push_constant_size;
	uint32_t push_constant_stages_mask;
	uint32_t stage_count;
	uint32_t shader_name_len;
};

// RenderingShaderContainer::ReflectionBindingData
struct ReflectionBindingData {
	uint32_t type;
	uint32_t binding;
	uint32_t stages;
	uint32_t length;
	uint32_t writable;
};

// RenderingShaderContainer::ReflectionSpecializationData
struct ReflectionSpecializationData {
	uint32_t type;
	uint32_t constant_id;
	uint32_t int_value;
	uint32_t stage_flags;
};

// RenderingShaderContainer::ShaderHeader
struct ShaderHeader {
	uint32_t shader_stage;
	uint32_t code_compressed_size;
	uint32_t code_compression_flags;
	uint32_t code_decompressed_size;
};

// Metal-specific extra data (drivers/metal/rendering_shader_container_metal.h).

// MetalDeviceProfile::Features
struct MetalProfileFeatures {
	uint32_t msl_version;
	bool use_argument_buffers;
	bool simdPermute;
};

// MetalDeviceProfile
struct MetalDeviceProfile {
	uint32_t platform;
	uint32_t gpu;
	uint32_t min_os_version;
	MetalProfileFeatures features;
};

// RenderingShaderContainerMetal::HeaderData
struct MetalHeaderData {
	MetalDeviceProfile profile;
	uint32_t msl_version;
	uint32_t os_min_version;
	uint32_t flags;
	uint32_t push_constant_binding;
};

// RenderingShaderContainerMetal::UniformData
struct MetalUniformData {
	uint32_t active_stages;
	uint32_t uniform_type;
	uint32_t data_type;
	uint32_t access;
	uint32_t usage;
	uint32_t texture_type;
	uint32_t image_format;
	uint32_t array_length;
	uint32_t is_multisampled;
	struct Indexes {
		uint32_t buffer;
		uint32_t texture;
		uint32_t sampler;
	};
	Indexes slot;
	Indexes arg_buffer;
};

// RenderingShaderContainerMetal::StageData
struct MetalStageData {
	uint32_t vertex_input_binding_mask;
	uint32_t is_position_invariant;
	uint32_t supports_fast_math;
	uint8_t hash[32]; // SHA256Digest
	uint32_t source_size;
	uint32_t library_size;
};

static_assert(sizeof(ContainerHeader) == 20);
static_assert(sizeof(ReflectionData) == 64); // 60 bytes data + 4 bytes trailing padding (uint64_t alignment)
static_assert(sizeof(ReflectionBindingData) == 20);
static_assert(sizeof(ReflectionSpecializationData) == 16);
static_assert(sizeof(ShaderHeader) == 16);
static_assert(sizeof(MetalHeaderData) == 36);
static_assert(sizeof(MetalUniformData) == 60);
static_assert(sizeof(MetalStageData) == 52);

#endif //GODOT_TYPES_H
