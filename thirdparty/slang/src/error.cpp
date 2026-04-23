//
// Created by Stuart Carnie on 6/8/2024.
//

#include "error.h"
#include "u32string.h"

namespace slang::error {

std::string Error::to_string() const {
	std::string result;
	switch (type) {
		case Type::PARAMETER_MISMATCH:
			result = u32::to_utf8(U"parameter mismatch: " + _parameter_mismatch);
			break;
		case Type::PATH_NOT_FOUND:
			result = u32::to_utf8(U"path not found: " + _path_not_found.generic_u32string());
			break;
		case Type::GLSL_COMPILE:
			result = "GLSL compile error:\n" + _utf8_string;
			break;
		case Type::FAILED_MSG:
			result = _utf8_string;
			break;
		case Type::PARSE:
			result = "parse error";
			break;
		case Type::FAILED:
			result = "failed";
			break;
	}

	return result;
}

} //namespace slang::error
