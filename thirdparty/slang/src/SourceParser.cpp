//
// Created by Stuart Carnie on 5/8/2024.
//

#include "SourceParser.h"
#include "scanner.h"
#include "u32string.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>

namespace slang {

namespace fs = std::filesystem;
using std::holds_alternative, std::get, std::u32string;

namespace prefixes {
static const u32string version = U"#version ";
static const u32string include = U"#include ";
static const u32string endif = U"#endif";
static const u32string pragma = U"#pragma ";
static const u32string pragma_name = U"#pragma name ";
static const u32string pragma_param = U"#pragma parameter ";
static const u32string pragma_format = U"#pragma format ";
static const u32string pragma_stage = U"#pragma stage ";
} //namespace prefixes

error::ErrorOpt SourceParser::load(fs::path p_source_path, bool p_is_root) {
	std::ifstream file(p_source_path);
	if (!file.is_open()) {
		return error::Error::path_not_found(p_source_path);
	}

	fs::path base_path = p_source_path.parent_path();

	u32string filename = p_source_path.filename().generic_u32string();

	std::string tmp;
	u32string line;
	int line_no = 1;
	if (p_is_root) {
		if (!std::getline(file, tmp)) {
			// missing
			return error::Error::parse();
		}

		line = u32::from_utf8(tmp);

		// line begins with prefixes::version
		if (!u32::starts_with(line, prefixes::version)) {
			return error::Error::parse();
		}
		_buffer.push_back(line);
		_buffer.push_back(U"#extension GL_GOOGLE_cpp_style_line_directive : require");
		line_no++;
	}

	_buffer.push_back(U"#line " + u32::from_int(line_no) + U" \"" + filename + U"\"");

	while (std::getline(file, tmp)) {
		line = u32::from_utf8(tmp);
		line_no++;
		if (u32::starts_with(line, prefixes::include)) {
			char32_t const *ptr = line.data() + prefixes::include.length();
			char32_t const *end = line.data() + line.length();

			u32string include_file = scan_quoted_string(ptr, end);

			fs::path include_path = fs::path(include_file);
			if (include_path.is_relative()) {
				include_path = canonical(base_path / include_path);
			}

			if (_included.find(include_path) == _included.end()) {
				auto err = load(include_path, false);
				if (err.has_value()) {
					return err;
				}
				_buffer.push_back(U"#line " + u32::from_int(line_no) + U" \"" + filename + U"\"");
				_included.insert(include_path);
			}
		} else {
			bool has_preprocessor;
			if (u32::starts_with(line, prefixes::pragma)) {
				has_preprocessor = true;
				auto res = process_pragma(line);
				if (const auto err = std::get_if<error::Error>(&res); err != nullptr) {
					return *err;
				} else if (get<bool>(res)) {
					// skip line, as it was processed
					continue;
				}
			} else if (u32::starts_with(line, prefixes::endif)) {
				has_preprocessor = true;
			} else {
				has_preprocessor = false;
			}

			_buffer.push_back(line);

			if (has_preprocessor) {
				_buffer.push_back(U"#line " + u32::from_int(line_no + 1) + U" \"" + filename + U"\"");
			}
		}

		line_no++;
	}

	return std::nullopt;
}

variant<bool, error::Error> SourceParser::process_pragma(const u32string &p_line) {
	const char32_t *ptr = p_line.data();
	const char32_t *end = ptr + p_line.length();

	if (u32::starts_with(p_line, prefixes::pragma_name)) {
		name = p_line.substr(prefixes::pragma_name.length(), p_line.length() - prefixes::pragma_name.length());
	} else if (u32::starts_with(p_line, prefixes::pragma_param)) {
		ptr += prefixes::pragma_param.length();
		u32string param_name = scan_identifier(ptr, end);
		if (param_name.empty()) {
			return error::Error::parse();
		}
		u32string desc = scan_quoted_string(ptr, end);

		double initial;
		if (auto val = scan_double(ptr, end); val.has_value()) {
			initial = val.value();
		} else {
			return error::Error::parse();
		}

		double minimum;
		if (auto val = scan_double(ptr, end); val.has_value()) {
			minimum = val.value();
		} else {
			return error::Error::parse();
		}

		double maximum;
		if (auto val = scan_double(ptr, end); val.has_value()) {
			maximum = val.value();
		} else {
			return error::Error::parse();
		}

		double step;
		if (auto val = scan_double(ptr, end); val.has_value()) {
			step = val.value();
		} else {
			step = 0.1 * (maximum - minimum);
		}
		parameters_map[param_name] = ShaderParameter(param_name, desc, initial, minimum, maximum, step);
		parameters.push_back(ShaderParameter(param_name, desc, initial, minimum, maximum, step));
	} else if (u32::starts_with(p_line, prefixes::pragma_format)) {
		if (format.has_value()) {
			return error::Error::parse();
		}
		ptr += prefixes::pragma_format.length();
		u32string format_str = scan_identifier(ptr, end);
		if (format_str.empty()) {
			return error::Error::parse();
		}
		format = compiled::pixel_format_from_string(format_str);
		if (!format.has_value()) {
			return error::Error::parse();
		}
	} else if (u32::starts_with(p_line, prefixes::pragma_stage)) {
		// ignore
		return false;
	}

	return true;
}

u32string SourceParser::get_source(const u32string &p_stage) {
	std::vector<u32string> src;

	bool keep = true;

	for (const u32string &line : _buffer) {
		if (u32::starts_with(line, prefixes::pragma_stage)) {
			const char32_t *ptr = line.data() + prefixes::pragma_stage.length();
			const char32_t *end = line.data() + line.length();
			u32string stage = scan_identifier(ptr, end);
			if (stage == p_stage) {
				keep = true;
			} else {
				keep = false;
			}
		} else if (u32::starts_with(line, prefixes::pragma_name) || u32::starts_with(line, prefixes::pragma_format)) {
			// skip
		} else if (keep) {
			src.push_back(line);
		}
	}

	// join lines with newlines
	return u32::joined(src, U"\n");
}

u32string &SourceParser::get_vert_source() {
	if (!_vert_source.has_value()) {
		_vert_source = get_source(U"vertex");
	}
	return _vert_source.value();
}

u32string &SourceParser::get_frag_source() {
	if (!_frag_source.has_value()) {
		_frag_source = get_source(U"fragment");
	}
	return _frag_source.value();
}

SourceParser::SourceParser(fs::path const &p_source_path, error::ErrorOpt &r_error) {
	_base_name = p_source_path.stem();
	auto err = load(p_source_path, true);
	if (err.has_value()) {
		r_error.emplace(err.value());
	}
}

} //namespace slang
