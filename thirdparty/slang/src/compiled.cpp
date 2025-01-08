//
// Created by Stuart Carnie on 5/8/2024.
//

#include "compiled.h"

#include <map>

namespace slang {
namespace compiled {

static std::map<std::u32string, PixelFormat> *str_to_pixel_format = nullptr;

void init_pixel_format_map() {
	str_to_pixel_format = new std::map<std::u32string, PixelFormat>{
		{ U"R8_UNORM",                 PixelFormat::r8Unorm         },
		{ U"R8_UINT",                  PixelFormat::r8Uint          },
		{ U"R8_SINT",                  PixelFormat::r8Sint          },
		{ U"R8G8_UNORM",               PixelFormat::rg8Unorm        },
		{ U"R8G8_UINT",                PixelFormat::rg8Uint         },
		{ U"R8G8_SINT",                PixelFormat::rg8Sint         },
		{ U"R8G8B8A8_UNORM",           PixelFormat::rgba8Unorm      },
		{ U"R8G8B8A8_UINT",            PixelFormat::rgba8Uint       },
		{ U"R8G8B8A8_SINT",            PixelFormat::rgba8Sint       },
		{ U"R8G8B8A8_SRGB",            PixelFormat::rgba8Unorm_srgb },
		{ U"A2B10G10R10_UNORM_PACK32", PixelFormat::rgb10a2Unorm    },
		{ U"A2B10G10R10_UINT_PACK32",  PixelFormat::rgb10a2Uint     },
		{ U"R16_UINT",                 PixelFormat::r16Uint         },
		{ U"R16_SINT",                 PixelFormat::r16Sint         },
		{ U"R16_SFLOAT",               PixelFormat::r16Float        },
		{ U"R16G16_UINT",              PixelFormat::rg16Uint        },
		{ U"R16G16_SINT",              PixelFormat::rg16Sint        },
		{ U"R16G16_SFLOAT",            PixelFormat::rg16Float       },
		{ U"R16G16B16A16_UINT",        PixelFormat::rgba16Uint      },
		{ U"R16G16B16A16_SINT",        PixelFormat::rgba16Sint      },
		{ U"R16G16B16A16_SFLOAT",      PixelFormat::rgba16Float     },
		{ U"R32_UINT",                 PixelFormat::r32Uint         },
		{ U"R32_SINT",                 PixelFormat::r32Sint         },
		{ U"R32_SFLOAT",               PixelFormat::r32Float        },
		{ U"R32G32_UINT",              PixelFormat::rg32Uint        },
		{ U"R32G32_SINT",              PixelFormat::rg32Sint        },
		{ U"R32G32_SFLOAT",            PixelFormat::rg32Float       },
		{ U"R32G32B32A32_UINT",        PixelFormat::rgba32Uint      },
		{ U"R32G32B32A32_SINT",        PixelFormat::rgba32Sint      },
		{ U"R32G32B32A32_SFLOAT",      PixelFormat::rgba32Float     },
	};
}

optional<PixelFormat> pixel_format_from_string(std::u32string const &p_str) {
	if (str_to_pixel_format == nullptr) {
		init_pixel_format_map();
	}

	auto iter = str_to_pixel_format->find(p_str);
	if (iter == str_to_pixel_format->end()) {
		return std::nullopt;
	}

	return iter->second;
}

Filter filter_from_bool(optional<bool> const &p_val) {
	if (p_val.has_value()) {
		return p_val.value() ? Filter::LINEAR : Filter::NEAREST;
	}
	return Filter::UNSPECIFIED;
}

Wrap wrap_mode_from_string(optional<std::u32string> const &p_val) {
	if (!p_val.has_value()) {
		return Wrap::BORDER;
	}

	if (p_val.value() == U"clamp_to_border") {
		return Wrap::BORDER;
	} else if (p_val.value() == U"clamp_to_edge") {
		return Wrap::EDGE;
	} else if (p_val.value() == U"repeat") {
		return Wrap::REPEAT;
	} else if (p_val.value() == U"mirrored_repeat") {
		return Wrap::BORDER;
	}

	// user specified an invalid value
	return Wrap::BORDER;
}

char const *to_cstr(Filter p_val) {
	switch (p_val) {
		case Filter::UNSPECIFIED:
			return "Unspecified";
		case Filter::LINEAR:
			return "Linear";
		case Filter::NEAREST:
			return "Nearest";
	}
}

char const *to_cstr(Wrap p_val) {
	switch (p_val) {
		case Wrap::BORDER:
			return "Border";
		case Wrap::EDGE:
			return "Edge";
		case Wrap::REPEAT:
			return "Repeat";
		case Wrap::MIRRORED_REPEAT:
			return "MirroredRepeat";
	}
}

char const *to_cstr(ShaderTextureSemantic p_val) {
	switch (p_val) {
		case ShaderTextureSemantic::ORIGINAL:
			return "Original";
		case ShaderTextureSemantic::SOURCE:
			return "Source";
		case ShaderTextureSemantic::ORIGINAL_HISTORY:
			return "OriginalHistory";
		case ShaderTextureSemantic::PASS_OUTPUT:
			return "PassOutput";
		case ShaderTextureSemantic::PASS_FEEDBACK:
			return "PassFeedback";
		case ShaderTextureSemantic::USER:
			return "User";
	}
}

const std::vector<ShaderTextureSemantic> texture_semantics() {
	return {
		ShaderTextureSemantic::ORIGINAL,
		ShaderTextureSemantic::SOURCE,
		ShaderTextureSemantic::ORIGINAL_HISTORY,
		ShaderTextureSemantic::PASS_OUTPUT,
		ShaderTextureSemantic::PASS_FEEDBACK,
		ShaderTextureSemantic::USER,
	};
};

const std::vector<ShaderBufferSemantic> buffer_semantics() {
	return {
		ShaderBufferSemantic::MVP,
		ShaderBufferSemantic::OUTPUT_SIZE,
		ShaderBufferSemantic::FINAL_VIEWPORT_SIZE,
		ShaderBufferSemantic::FRAME_COUNT,
		ShaderBufferSemantic::FRAME_DIRECTION,
		ShaderBufferSemantic::FLOAT_PARAMETER,
		ShaderBufferSemantic::ORIGINAL_SIZE,
		ShaderBufferSemantic::SOURCE_SIZE,
		ShaderBufferSemantic::ORIGINAL_HISTORY_SIZE,
		ShaderBufferSemantic::PASS_OUTPUT_SIZE,
		ShaderBufferSemantic::PASS_FEEDBACK_SIZE,
		ShaderBufferSemantic::USER_SIZE,
	};
}

std::string to_string(ShaderTextureSemantic p_val) {
	return to_cstr(p_val);
}

char const *to_cstr(ShaderBufferSemantic p_val) {
	switch (p_val) {
		case ShaderBufferSemantic::MVP:
			return "MVP";
		case ShaderBufferSemantic::OUTPUT_SIZE:
			return "OutputSize";
		case ShaderBufferSemantic::FINAL_VIEWPORT_SIZE:
			return "FinalViewportSize";
		case ShaderBufferSemantic::FRAME_COUNT:
			return "FrameCount";
		case ShaderBufferSemantic::FRAME_DIRECTION:
			return "FrameDirection";
		case ShaderBufferSemantic::FLOAT_PARAMETER:
			return "FloatParameter";
		case ShaderBufferSemantic::ORIGINAL_SIZE:
			return "OriginalSize";
		case ShaderBufferSemantic::SOURCE_SIZE:
			return "SourceSize";
		case ShaderBufferSemantic::ORIGINAL_HISTORY_SIZE:
			return "OriginalHistorySize";
		case ShaderBufferSemantic::PASS_OUTPUT_SIZE:
			return "PassOutputSize";
		case ShaderBufferSemantic::PASS_FEEDBACK_SIZE:
			return "PassFeedbackSize";
		case ShaderBufferSemantic::USER_SIZE:
			return "UserSize";
	}
}

std::string to_string(ShaderBufferSemantic p_val) {
	return to_cstr(p_val);
}

} //namespace compiled
} //namespace slang
