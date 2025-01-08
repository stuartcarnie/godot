//
// Created by Stuart Carnie on 4/8/2024.
//

#include "ConfigFile.h"
#include "scanner.h"
#include "u32string.h"

#include <unicode/uchar.h>
#include <fstream>
#include <iostream>
#include <optional>

namespace slang {

using std::string;

error::ErrorOpt ConfigFile::read(const fs::path &p_path) {
	std::ifstream file(p_path);
	if (!file.is_open()) {
		return error::Error::path_not_found(p_path);
	}

	string line;
	while (std::getline(file, line)) {
		u32string str = u32::from_utf8(line);
		if (str.empty()) {
			continue;
		}

		char32_t const *ptr = str.data();
		char32_t const *end = ptr + str.length();

		ptr = skip_whitespace(ptr, end);
		if (ptr == end) {
			continue;
		}

		// parse key
		u32string key = scan_identifier(ptr, end);
		ptr = skip_whitespace(ptr, end);

		if (ptr == end || *ptr != U'=') {
			continue;
		}

		ptr++; // skip '='
		ptr = skip_whitespace(ptr, end);
		if (ptr == end) {
			continue;
		}

		// scan value, which could be quoted
		u32string value;
		if (*ptr == U'"') {
			value = scan_quoted_string(ptr, end);
		} else {
			char32_t const *value_start = ptr;
			while (ptr != end && !u_isspace(*ptr) && *ptr != U'#') {
				ptr++;
			}
			value = u32string(value_start, ptr - value_start);
		}

		_data[key] = value;
	}

	return std::nullopt;
}

} //namespace slang
