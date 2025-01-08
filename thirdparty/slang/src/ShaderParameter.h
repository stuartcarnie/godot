//
// Created by Stuart Carnie on 6/8/2024.
//

#ifndef GODOT_SHADERPARAMETER_H
#define GODOT_SHADERPARAMETER_H

#include <string>

namespace slang {

struct ShaderParameter {
	std::u32string name;
	std::u32string desc;
	double initial = 0.0;
	double minimum = 0.0;
	double maximum = 1.0;
	double step = 0.01;

	ShaderParameter() {}
	ShaderParameter(const std::u32string &p_name, const std::u32string &p_desc, double p_initial, double p_min, double p_max, double p_step) :
			name(p_name), desc(p_desc), initial(p_initial), minimum(p_min), maximum(p_max), step(p_step) {}

	bool operator==(const ShaderParameter &p_other) const {
		return name == p_other.name &&
				desc == p_other.desc &&
				initial == p_other.initial &&
				minimum == p_other.minimum &&
				maximum == p_other.maximum &&
				step == p_other.step;
	}

	bool operator!=(const ShaderParameter &p_other) const {
		return !(*this == p_other);
	}
};

} //namespace slang

#endif //GODOT_SHADERPARAMETER_H
