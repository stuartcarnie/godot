//
// Created by Stuart Carnie on 6/8/2024.
//

#pragma once

#include <string>
#include <string_view>
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
std::string to_ascii(const std::u32string &p_str, char replacement = '?');
constexpr bool starts_with(std::u32string_view p_str, std::u32string_view p_prefix) {
	return p_prefix.length() <= p_str.length() && p_str.compare(0, p_prefix.length(), p_prefix) == 0;
}
std::u32string joined(const std::vector<std::u32string> &p_strs, std::u32string_view p_delimiter);
} //namespace u32
