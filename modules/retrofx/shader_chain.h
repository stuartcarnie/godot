//
// Created by Stuart Carnie on 1/1/2025.
//

#pragma once

#include "filter_chain.h"

#include "core/object/ref_counted.h"
#include "scene/main/viewport.h"

class ShaderChain;

class ShaderParameter : public RefCounted {
	GDCLASS(ShaderParameter, RefCounted);

	uint32_t index = 0;
	String name;
	String desc;
	double initial = 0;
	double minimum = 0;
	double maximum = 0;
	double step = 0;

protected:
	static void _bind_methods();

public:
	// Getters
	uint32_t get_index() const { return index; }
	/// Get the name of the parameter.
	String get_name() const { return name; }
	/// Get the description of the parameter.
	String get_desc() const { return desc; }
	/// Get the initial or default value of the parameter.
	double get_initial() const { return initial; }
	/// Get the minimum value of the parameter.
	double get_minimum() const { return minimum; }
	/// Get the maximum value of the parameter.
	double get_maximum() const { return maximum; }
	/// Get the step value of the parameter.
	double get_step() const { return step; }

	/// Return true if the parameter is a boolean type.
	bool is_boolean() const { return minimum == 0 && maximum == 1 && step == 1; }

	/// Return true if the parameter is an integer type.
	bool is_integer() const {
		return Math::is_equal_approx(step, 1.0) && Math::is_equal_approx(Math::round(minimum), minimum) && Math::is_equal_approx(Math::round(maximum), maximum);
	}

	ShaderParameter() = default;

	ShaderParameter(const compiled::Parameter &p_param) {
		index = p_param.index;
		name = p_param.name.c_str();
		desc = p_param.desc.c_str();
		initial = p_param.initial;
		minimum = p_param.minimum;
		maximum = p_param.maximum;
		step = p_param.step;
	}
	~ShaderParameter() override {}
};

class ShaderChain : public RefCounted {
	GDCLASS(ShaderChain, RefCounted);

	FilterChain *filter_chain;
	HashMap<String, uint32_t> parameter_name_to_index;
	TypedArray<ShaderParameter> parameters;

protected:
	static void _bind_methods();

public:
	Error load_from_file(const String &p_path);

	void set_source_rect(Rect2i p_rect, Size2i p_aspect) { filter_chain->set_source_rect(p_rect, p_aspect); }
	Rect2 get_source_rect() const { return filter_chain->get_source_rect(); }

	void set_drawable_size(Size2i p_size) { filter_chain->set_drawable_size(p_size); }
	Size2 get_drawable_size() const { return filter_chain->get_drawable_size(); }

	Rect2 get_output_bounds() const { return filter_chain->get_output_bounds(); }

	TypedArray<ShaderParameter> get_parameters() const;

	Ref<ShaderParameter> get_parameter_by_index(uint32_t p_index) const { return parameters[p_index]; }
	Ref<ShaderParameter> get_parameter_by_name(const String &p_name) const {
		if (const uint32_t *index = parameter_name_to_index.getptr(p_name)) {
			return parameters[*index];
		}
		return Ref<ShaderParameter>();
	}
	void set_parameter_value_by_index(uint32_t p_index, double p_value) { filter_chain->set_parameter_value(p_index, p_value); }
	void set_parameter_value_by_name(const String &p_name, double p_value) { filter_chain->set_parameter_value(p_name, p_value); }
	bool get_parameter_value_by_index(uint32_t p_index, double &r_value) const { return filter_chain->get_parameter_value(p_index, r_value); }
	bool get_parameter_value_by_name(const String &p_name, double &r_value) const { return filter_chain->get_parameter_value(p_name, r_value); }
	bool get_default_parameter_value_by_index(uint32_t p_index, double &r_value) const;
	bool get_default_parameter_value_by_name(const String &p_name, double &r_value) const;

	void render(const RID p_source, const Size2i p_source_size, const RID p_target, const Size2i p_target_size) {
		filter_chain->render(p_source, p_source_size, p_target, p_target_size);
	}

	bool has_shader_loaded() const { return filter_chain->has_shader_loaded(); }

	ShaderChain();
	~ShaderChain() override;
};
