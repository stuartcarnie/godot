//
// Created by Stuart Carnie on 5/8/2024.
//

#include "scanner.h"
#include <unicode/uchar.h>

namespace slang {

char32_t const *skip_whitespace(char32_t const *p_str, char32_t const *p_end) {
	while (p_str != p_end && u_isspace(*p_str)) {
		p_str++;
	}

	if (p_str == p_end) {
		return p_str;
	}

	// skip comments
	if (*p_str == '#') {
		return p_end;
	}

	if (p_end - p_str >= 2 && *p_str == '/' && *(p_str + 1) == '/') {
		return p_end;
	}

	return p_str;
}

static bool is_identifier(char32_t c) {
	return u_isalnum(c) || c == U'_' || c == U'-';
}

std::u32string scan_identifier(char32_t const *&ptr, char32_t const *end) {
	ptr = skip_whitespace(ptr, end);
	char32_t const *id_start = ptr;
	while (ptr != end && is_identifier(*ptr)) {
		ptr++;
	}
	return std::u32string(id_start, ptr - id_start);
}

std::u32string scan_quoted_string(char32_t const *&ptr, char32_t const *end) {
	ptr = skip_whitespace(ptr, end);
	if (ptr == end || *ptr != U'"') {
		return std::u32string();
	}

	ptr++;
	char32_t const *str_start = ptr;
	while (ptr != end && *ptr != U'"') {
		ptr++;
	}

	if (ptr == end) {
		return std::u32string();
	}

	ptr++;
	return std::u32string(str_start, ptr - str_start - 1);
}

char32_t const *skip_to_whitespace(char32_t const *ptr, char32_t const *end) {
	while (ptr != end && !u_isspace(*ptr)) {
		ptr++;
	}
	return ptr;
}

optional<double> scan_double(char32_t const *&ptr, char32_t const *end) {
	ptr = skip_whitespace(ptr, end);

	double sign = 1;
	if (ptr != end) {
		if (*ptr == U'-') {
			sign = -1;
			ptr++;
		} else if (*ptr == U'+') {
			ptr++;
		}
	}

	if (ptr == end || !(u_isdigit(*ptr) || *ptr == U'.')) {
		// if we don't have a digit or a dot, it's not a number
		return std::nullopt;
	}

	double whole = 0;
	while (ptr != end && u_isdigit(*ptr)) {
		whole = whole * 10 + (*ptr - U'0');
		ptr++;
	}

	if (ptr != end && *ptr == U'.') {
		double frac = 0;
		double scale = 1;
		ptr++;
		while (ptr != end && u_isdigit(*ptr)) {
			frac = frac * 10 + (*ptr - U'0');
			scale *= 10;
			ptr++;
		}

		return sign * (whole + frac / scale);
	}

	return sign * whole;
}

} //namespace slang
