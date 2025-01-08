//
// Created by Stuart Carnie on 6/8/2024.
//

#ifndef SLANG_U32STRING_H
#define SLANG_U32STRING_H

#include <string>
#include <vector>

namespace u32 {
int to_int(std::u32string const &p_str);
double to_double(std::u32string const &p_str);

static std::u32string from_int(int p_value) {
	auto v = std::to_wstring(p_value);
	return std::u32string(v.begin(), v.end());
}

static std::u32string from_double(double p_value) {
	auto v = std::to_wstring(p_value);
	return std::u32string(v.begin(), v.end());
}

std::vector<std::u32string> split(const std::u32string &p_str, char32_t p_delimiter);
std::u32string from_utf8(const std::string &p_str);
std::string to_utf8(const std::u32string &p_str);
bool starts_with(const std::u32string &p_str, const std::u32string &p_prefix);
std::u32string joined(const std::vector<std::u32string> &p_strs, std::u32string const &p_delimiter);
} //namespace u32

#endif //SLANG_U32STRING_H
