//
// Created by Stuart Carnie on 1/1/2025.
//

#pragma once

#include "filter_chain.h"

#include "core/object/gdvirtual.gen.inc"
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
	String get_name() const { return name; }
	String get_desc() const { return desc; }
	double get_initial() const { return initial; }
	double get_minimum() const { return minimum; }
	double get_maximum() const { return maximum; }
	double get_step() const { return step; }

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
	~ShaderParameter() {}
};

class ShaderChain : public RefCounted {
	GDCLASS(ShaderChain, RefCounted);

	FilterChain *filter_chain;
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
	void set_parameter_value_by_index(uint32_t p_index, double p_value) { filter_chain->set_parameter_value(p_index, p_value); }
	void set_parameter_value_by_name(const String &p_name, double p_value) { filter_chain->set_parameter_value(p_name, p_value); }

	void render(const RID p_source, const Size2i p_source_size, const RID p_target, const Size2i p_target_size) {
		filter_chain->render(p_source, p_source_size, p_target, p_target_size);
	}

	bool has_shader_loaded() const { return filter_chain->has_shader_loaded(); }

	ShaderChain();
	~ShaderChain();
};
