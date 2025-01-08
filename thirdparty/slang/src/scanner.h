//
// Created by Stuart Carnie on 5/8/2024.
//

#ifndef GODOT_SCANNER_H
#define GODOT_SCANNER_H

#include <optional>
#include <string>

namespace slang {

using std::optional;

char32_t const *skip_whitespace(char32_t const *p_str, char32_t const *p_end);
std::u32string scan_identifier(char32_t const *&ptr, char32_t const *end);
std::u32string scan_quoted_string(char32_t const *&ptr, char32_t const *end);
optional<double> scan_double(char32_t const *&ptr, char32_t const *end);

} //namespace slang

#endif //GODOT_SCANNER_H
