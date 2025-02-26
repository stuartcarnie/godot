//
// Created by Stuart Carnie on 5/8/2024.
//

#pragma once

#include "ShaderParameter.h"
#include "compiled.h"
#include "error.h"

#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include <variant>
#include <vector>

namespace slang {

namespace fs = std::filesystem;
using std::variant, std::optional;

class SourceParser {
	std::vector<std::u32string> _buffer;
	std::set<fs::path> _included;
	fs::path _base_name;

	error::ErrorOpt load(fs::path p_source_path, bool p_is_root);
	variant<bool, error::Error> process_pragma(const std::u32string &p_line);

	optional<std::u32string> _vert_source;
	optional<std::u32string> _frag_source;

	std::u32string get_source(const std::u32string &p_stage);

public:
	optional<std::u32string> name;
	optional<compiled::PixelFormat> format;

	std::map<std::u32string, ShaderParameter> parameters_map;
	std::vector<ShaderParameter> parameters;

	/**
	 * Returns the vertex source code.
	 * @return
	 */
	std::u32string &get_vert_source();

	/**
	 * Returns the fragment source code.
	 * @return
	 */
	std::u32string &get_frag_source();

	SourceParser() {};
	SourceParser(fs::path const &p_source_path, error::ErrorOpt &r_error);

	~SourceParser() {};
};

} //namespace slang
