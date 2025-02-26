//
// Created by Stuart Carnie on 5/8/2024.
//

#pragma once

#include <optional>
#include <string>
#include <vector>

using std::optional;

namespace slang {
namespace compiled {
enum class ScaleType {
	Source,
	Absolute,
	Viewport
};

struct Scale {
private:
	Scale(ScaleType p_type, double p_val) :
			type(p_type), scale(p_val) {}
	Scale(ScaleType p_type, int p_val) :
			type(p_type), size(p_val) {}

public:
	ScaleType type;
	union {
		int size;
		double scale;
	};

	static Scale source(float p_val) {
		return Scale(ScaleType::Source, p_val);
	}

	static Scale absolute(int p_val) {
		return Scale(ScaleType::Absolute, p_val);
	}

	static Scale viewport(float p_val) {
		return Scale(ScaleType::Viewport, p_val);
	}

	bool operator==(const Scale &other) const {
		if (type != other.type) {
			return false;
		}

		switch (type) {
			case ScaleType::Source:
				return scale == other.scale;
			case ScaleType::Absolute:
				return size == other.size;
			case ScaleType::Viewport:
				return scale == other.scale;
			default:
				return false;
		}
	}
};

enum class Filter {
	UNSPECIFIED,
	LINEAR,
	NEAREST,
	MAX,
};

constexpr int to_int(Filter p_val) {
	return static_cast<int>(p_val);
}

constexpr Filter &operator++(Filter &p_val) {
	p_val = static_cast<Filter>(static_cast<int>(p_val) + 1);
	return p_val;
}

enum class Wrap {
	BORDER,
	EDGE,
	REPEAT,
	MIRRORED_REPEAT,
	MAX,
};

constexpr int to_int(Wrap p_val) {
	return static_cast<int>(p_val);
}

constexpr Wrap &operator++(Wrap &p_val) {
	p_val = static_cast<Wrap>(static_cast<int>(p_val) + 1);
	return p_val;
}

enum class PixelFormat {
	r8Unorm,
	r8Uint,
	r8Sint,
	rg8Unorm,
	rg8Uint,
	rg8Sint,
	rgba8Unorm,
	rgba8Uint,
	rgba8Sint,
	rgba8Unorm_srgb,
	rgb10a2Unorm,
	rgb10a2Uint,
	r16Uint,
	r16Sint,
	r16Float,
	rg16Uint,
	rg16Sint,
	rg16Float,
	rgba16Uint,
	rgba16Sint,
	rgba16Float,
	r32Uint,
	r32Sint,
	r32Float,
	rg32Uint,
	rg32Sint,
	rg32Float,
	rgba32Uint,
	rgba32Sint,
	rgba32Float,
	bgra8Unorm_srgb,
	bgra8Unorm,
};

enum class ShaderTextureSemantic {
	/// Identifies the input texture to the filter chain.
	///
	/// Shaders refer to the input texture via the `Original` symbol.
	ORIGINAL,

	/// Identifies the output texture from the previous pass.
	///
	/// Shaders can refer to the previous source texture via
	/// the `Source` symbol.
	///
	/// - Note: If the filter chain is executing the first pass, this is the same as
	/// `Original`.
	SOURCE,

	/// Identifies the historical input textures.
	///
	/// Shaders can refer to the history textures via the
	/// `OriginalHistoryN` symbols, where `N`
	/// specifies the number of `Original` frames back to read.
	///
	/// - Note: To read 2 frames prior, use `OriginalHistory2`.
	ORIGINAL_HISTORY,

	/// Identifies the pass output textures.
	///
	/// Shaders can refer to the output of prior passes via the
	/// `PassOutputN` symbols, where `N` specifies the
	/// pass number.
	///
	/// - NOTE: In pass 5, sampling the output of pass 2
	/// would use `PassOutput2`.
	PASS_OUTPUT,

	/// Identifies the pass feedback textures.
	///
	/// Shaders can refer to the output of the previous
	/// frame of pass `N` via the `PassFeedbackN`
	/// symbols, where `N` specifies the pass number.
	///
	/// - NOTE: To sample the output of pass 2 from the prior frame,
	/// use `PassFeedback2`.
	PASS_FEEDBACK,

	/// Identifies the lookup or user textures.
	///
	/// Shaders refer to user lookup or user textures by name as defined
	/// in the `.slangp` file.
	USER,
};

const std::vector<ShaderTextureSemantic> texture_semantics();

std::string to_string(ShaderTextureSemantic p_semantic);
char const *to_cstr(ShaderTextureSemantic p_val);

enum class ShaderBufferSemantic {
	/// Identifies the 4x4 float model-view-projection matrix buffer.
	///
	/// Shaders refer to the matrix constant via the `MVP` symbol.
	///
	MVP,

	/// Identifies the vec4 float containing the viewport size of the current pass.
	///
	/// Shaders refer to the viewport size constant via the `OutputSize` symbol.
	///
	/// - NOTE: The `x` and `y` fields refer to the size of the output in pixels.
	/// The `z` and `w` fields refer to the inverse; `1/x` and `1/y`.
	OUTPUT_SIZE,

	/// Identifies the vec4 float containing the final viewport output size.
	///
	/// Shaders refer to the final output size constant via the `FinalViewportSize` symbol.
	///
	/// - NOTE: The `x` and `y` fields refer to the size of the output in pixels.
	/// The `z` and `w` fields refer to the inverse; `1/x` and `1/y`.
	FINAL_VIEWPORT_SIZE,

	/// Identifies the uint containing the frame count.
	///
	/// Shaders refer to the frame count constant via the `FrameCount` symbol.
	///
	/// - NOTE: This value increments by one each frame.
	FRAME_COUNT,

	/// Identifies the int containing the frame direction; 1 is forward, -1 is backwards.
	///
	/// Shaders refer to the frame direction constant via the `FrameDirection` symbol.
	FRAME_DIRECTION,

	/// Identifies a float parameter buffer.
	///
	/// Shaders refer to float parameters by name.
	FLOAT_PARAMETER,

	/// Identifies the input texture size to the filter chain.
	///
	/// Shaders refer to the input texture size via the `OriginalSize` symbol.
	ORIGINAL_SIZE,

	/// Identifies the output texture size from the previous pass.
	///
	/// Shaders can refer to the previous source texture size via
	/// the `SourceSize` symbol.
	///
	/// - Note: If the filter chain is executing the first pass, this is the same as
	/// `OriginalSize`.
	SOURCE_SIZE,

	/// Identifies the historical input texture sizes.
	///
	/// Shaders can refer to the history texture sizes via the
	/// `OriginalSizeN` symbols, where `N`
	/// specifies the number of frames back to read.
	///
	/// - Note: To read 2 frames prior, use `OriginalHistorySize2`.
	ORIGINAL_HISTORY_SIZE,

	/// Identifies the pass output texture sizes.
	///
	/// Shaders can refer to the output texture sizes of prior passes via the
	/// `PassOutputSizeN` symbols, where `N` specifies the
	/// pass number.
	///
	/// - NOTE: In pass 5, sampling the output of pass 2
	/// would use `PassOutputSize2`.
	PASS_OUTPUT_SIZE,

	/// Identifies the pass feedback texture sizes.
	///
	/// Shaders can refer to the output of the previous
	/// frame of pass `N` via the `PassFeedbackSizeN`
	/// symbols, where `N` specifies the pass number.
	///
	/// - NOTE: To sample the output of pass 2 from the prior frame,
	/// use `PassFeedbackSize2`.
	PASS_FEEDBACK_SIZE,

	/// Identifies the lookup or user texture sizes.
	///
	/// Shaders refer to user lookup or user textures by name as defined
	/// in the `.slangp` file.
	USER_SIZE,
};

const std::vector<ShaderBufferSemantic> buffer_semantics();

std::string to_string(ShaderBufferSemantic p_semantic);
char const *to_cstr(ShaderBufferSemantic p_val);

/// An object that describes how to store uniform data in memory
/// and map it to the arguments of a shader pass.
struct BufferUniformDescriptor {
	ShaderBufferSemantic semantic;
	/// An optional index if the uniform is an array
	optional<int> index;
	std::string name;
	int size = 0;
	int offset = 0;

	BufferUniformDescriptor() {}
	BufferUniformDescriptor(ShaderBufferSemantic p_semantic, optional<int> p_index, std::string p_name, int p_size, int p_offset) :
			semantic(p_semantic), index(p_index), name(p_name), size(p_size), offset(p_offset) {}
};

struct TextureDescriptor {
	std::string name;
	ShaderTextureSemantic semantic = ShaderTextureSemantic::ORIGINAL;
	int binding = 0;
	Wrap wrap = Wrap::REPEAT;
	Filter filter = Filter::UNSPECIFIED;
	int index = 0;

	TextureDescriptor() {}
	TextureDescriptor(std::string p_name, ShaderTextureSemantic p_semantic, int p_binding, Wrap p_wrap, Filter p_filter, int p_index) :
			name(p_name), semantic(p_semantic), binding(p_binding), wrap(p_wrap), filter(p_filter), index(p_index) {}
};

enum Stage {
	STAGE_NONE = 0,
	STAGE_VERTEX = 1 << 0,
	STAGE_FRAGMENT = 1 << 1,
};

/// An object that describes how to organize and map data
/// to a shader pass buffer.
struct UBOBufferDescriptor {
	uint32_t binding = -1u;
	Stage stage = Stage::STAGE_NONE;
	int size = 0;
	std::vector<BufferUniformDescriptor> uniforms;

	bool is_valid() const {
		return size > 0 && binding != -1u;
	}

	UBOBufferDescriptor() {}
	UBOBufferDescriptor(uint32_t p_binding, Stage p_stage, int p_size, std::vector<BufferUniformDescriptor> p_uniforms) :
			binding(p_binding), stage(p_stage), size(p_size), uniforms(p_uniforms) {}
};

struct PushBufferDescriptor {
	Stage stage = Stage::STAGE_NONE;
	int size = 0;
	std::vector<BufferUniformDescriptor> uniforms;

	bool is_valid() const {
		return size > 0;
	}

	PushBufferDescriptor() {}
	PushBufferDescriptor(Stage p_stage, int p_size, std::vector<BufferUniformDescriptor> p_uniforms) :
			stage(p_stage), size(p_size), uniforms(p_uniforms) {}
};

struct OutputFormat {
	optional<PixelFormat> format;
	bool is_float = false;
	bool is_sRGB = false;
};

struct ShaderPass {
	int index;
	std::string vertex_source;
	std::string fragment_source;
	uint32_t frame_count_mod;
	optional<Scale> scale_x;
	optional<Scale> scale_y;
	Filter filter;
	Wrap wrap_mode;
	OutputFormat format;
	bool is_feedback;
	UBOBufferDescriptor ubo;
	PushBufferDescriptor push;
	std::vector<TextureDescriptor> textures;
	optional<std::string> alias;
};

struct LUT {
	std::string url;
	std::string name;
	Filter filter;
	Wrap wrap_mode;
	bool is_mipmap;
};

struct Parameter {
	int index;
	std::string name;
	std::string desc;
	double initial;
	double minimum;
	double maximum;
	double step;
};

struct Shader {
	std::vector<ShaderPass> passes;
	std::vector<Parameter> parameters;
	std::vector<LUT> luts;
	int history_count;
};

optional<PixelFormat> pixel_format_from_string(std::u32string const &p_str);
Filter filter_from_bool(optional<bool> const &p_val);
Wrap wrap_mode_from_string(optional<std::u32string> const &p_val);

#pragma mark - Hashers for enums

struct Hashers {
	inline static uint32_t hash(const ShaderTextureSemantic p_semantic) {
		return static_cast<uint32_t>(p_semantic);
	}

	inline static uint32_t hash(const ShaderBufferSemantic p_semantic) {
		return static_cast<uint32_t>(p_semantic);
	}
};

} //namespace compiled
} //namespace slang
