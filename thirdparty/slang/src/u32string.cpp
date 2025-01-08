//
// Created by Stuart Carnie on 6/8/2024.
//

#include "u32string.h"
#include "thirdparty/simdutf/simdutf.h"

namespace u32 {
int to_int(std::u32string const &p_str) {
	size_t expected = simdutf::utf8_length_from_utf32(p_str.data(), p_str.length());
	if (expected == 0 || expected > 128) {
		return 0;
	}
	char *buf = (char *)alloca(expected + 1);
	size_t written = simdutf::convert_utf32_to_utf8(p_str.data(), p_str.length(), buf);
	if (written == 0) {
		return 0;
	}
	buf[written] = 0;
	return std::stoi(buf);
}

double to_double(std::u32string const &p_str) {
	size_t expected = simdutf::utf8_length_from_utf32(p_str.data(), p_str.length());
	if (expected == 0 || expected > 128) {
		return 0;
	}
	char *buf = (char *)alloca(expected + 1);
	size_t written = simdutf::convert_utf32_to_utf8(p_str.data(), p_str.length(), buf);
	if (written == 0) {
		return 0;
	}
	buf[written] = 0;
	return std::stod(buf);
}

std::vector<std::u32string> split(const std::u32string &p_str, char32_t p_delimiter) {
	std::vector<std::u32string> result;
	std::u32string::size_type start = 0;
	std::u32string::size_type end = 0;

	while ((end = p_str.find(p_delimiter, start)) != std::u32string::npos) {
		result.push_back(p_str.substr(start, end - start));
		start = end + 1;
	}
	result.push_back(p_str.substr(start));

	return result;
}

std::u32string from_utf8(const std::string &p_str) {
	size_t estimated = simdutf::utf32_length_from_utf8(p_str.data(), p_str.length());
	std::u32string str;
	str.resize(estimated + 1);
	size_t str_len = simdutf::convert_utf8_to_utf32(p_str.data(), p_str.length(), str.data());
	str.resize(str_len);
	return str;
}

std::string to_utf8(const std::u32string &p_str) {
	size_t estimated = simdutf::utf8_length_from_utf32(p_str.data(), p_str.length());
	std::string str;
	str.resize(estimated + 1);
	size_t str_len = simdutf::convert_utf32_to_utf8(p_str.data(), p_str.length(), str.data());
	str.resize(str_len);
	return str;
}

bool starts_with(const std::u32string &p_str, const std::u32string &p_prefix) {
	return p_prefix.length() <= p_str.length() && p_str.compare(0, p_prefix.length(), p_prefix) == 0;
}

std::u32string joined(const std::vector<std::u32string> &p_strs, std::u32string const &p_delimiter) {
	if (p_strs.empty()) {
		return std::u32string();
	}

	// count length of string required, including newlines
	size_t len = 0;
	for (const auto &line : p_strs) {
		len += line.length();
	}
	len += p_delimiter.length() * (p_strs.size() - 1);
	std::u32string result;
	result.reserve(len + 1);
	result += p_strs[0];
	for (size_t i = 1; i < p_strs.size(); i++) {
		result += p_delimiter;
		result += p_strs[i];
	}
	return result;
}

} //namespace u32
