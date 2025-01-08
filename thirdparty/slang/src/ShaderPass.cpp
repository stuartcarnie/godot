//
// Created by Stuart Carnie on 6/8/2024.
//

#include "ShaderPass.h"

namespace slang {

bool is_valid_scale(const ShaderPassModel &p_pass) {
	return
			// Either the shader pass specifies a scale_type for both axes
			p_pass.scale_type.has_value() ||
			// or individual scale type for the X and Y axis
			(p_pass.scale_type_x.has_value() && p_pass.scale_type_y.has_value());
}

optional<compiled::Scale> read_scale(ScaleAxis p_axis, const ShaderPassModel &p_pass) {
	std::u32string scale_type;
	if (p_pass.scale_type.has_value()) {
		scale_type = p_pass.scale_type.value();
	} else if (const optional<std::u32string> &scale_type_for_axis = p_pass.scale_type_for_axis(p_axis); scale_type_for_axis.has_value()) {
		scale_type = scale_type_for_axis.value();
	} else {
		return std::nullopt;
	}

	optional<double> val;
	if (p_pass.scale.has_value()) {
		val = p_pass.scale;
	} else {
		val = p_pass.scale_value(p_axis);
	}

	if (scale_type == U"source") {
		return compiled::Scale::source(val.value_or(1));
	} else if (scale_type == U"viewport") {
		return compiled::Scale::viewport(val.value_or(1));
	} else if (scale_type == U"absolute") {
		return compiled::Scale::absolute(round(val.value_or(0)));
	}

	return std::nullopt;
}

ShaderPass::ShaderPass(fs::path const &p_path, const ShaderPassModel &p_pass) {
	path = p_path;
	index = p_pass.pass;
	filter = compiled::filter_from_bool(p_pass.filter_linear);
	wrap_mode = compiled::wrap_mode_from_string(p_pass.wrap_mode);
	frame_count_mod = p_pass.frame_count_mod.value_or(0);
	is_sRGB = p_pass.srgb_framebuffer.value_or(false);
	is_float = p_pass.float_framebuffer.value_or(false);
	is_mipmap = p_pass.mipmap_input.value_or(false);

	if (is_valid_scale(p_pass)) {
		scale_x = read_scale(ScaleAxis::X, p_pass);
		scale_y = read_scale(ScaleAxis::Y, p_pass);
	} else {
		scale_x = std::nullopt;
		scale_y = std::nullopt;
	}

	error::ErrorOpt err;
	_parser = SourceParser(p_path, err);

	if (p_pass.alias.has_value()) {
		alias = p_pass.alias.value();
	} else {
		alias = _parser.name;
	}
}

} // namespace slang
