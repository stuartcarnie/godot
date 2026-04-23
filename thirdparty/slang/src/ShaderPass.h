//
// Created by Stuart Carnie on 6/8/2024.
//

#pragma once

#include "ShaderModel.h"
#include "SourceParser.h"

#include <filesystem>
#include <optional>
#include <string>

namespace slang {

namespace fs = std::filesystem;
using std::optional;

struct ShaderPass {
	SourceParser _parser;

	fs::path path;
	int index;
	uint32_t frame_count_mod;
	optional<compiled::Scale> scale_x;
	optional<compiled::Scale> scale_y;
	compiled::Filter filter;
	compiled::Wrap wrap_mode;
	bool is_float;
	bool is_sRGB;
	bool is_mipmap;
	optional<std::u32string> alias;

	optional<compiled::PixelFormat> pixel_format() const {
		if (_parser.format.has_value()) {
			return _parser.format.value();
		}

		if (is_sRGB) {
			return compiled::PixelFormat::bgra8Unorm_srgb;
		}

		if (is_float) {
			return compiled::PixelFormat::rgba16Float;
		}

		return compiled::PixelFormat::bgra8Unorm;
	}

	ShaderPass(fs::path const &p_path, const ShaderPassModel &p_model);
	ShaderPass() :
			index(0),
			frame_count_mod(0),
			filter(compiled::Filter::UNSPECIFIED),
			wrap_mode(compiled::Wrap::BORDER),
			is_float(false),
			is_sRGB(false),
			is_mipmap(false) {}
};

} // namespace slang
