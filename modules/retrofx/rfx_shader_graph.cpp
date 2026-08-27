//
// Created by Stuart Carnie on 8/8/2024.
//

#include "rfx_shader_graph.h"

#include "core/object/class_db.h"
#include "core/variant/typed_array.h"

void RFXShaderGraph::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_shader_passes", "passes"), &RFXShaderGraph::set_shader_passes);
	ClassDB::bind_method(D_METHOD("get_shader_passes"), &RFXShaderGraph::get_shader_passes);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "passes", PROPERTY_HINT_ARRAY_TYPE, MAKE_RESOURCE_TYPE_HINT("ShaderPass")), "set_shader_passes", "get_shader_passes");

	ClassDB::bind_method(D_METHOD("set_luts", "passes"), &RFXShaderGraph::set_luts);
	ClassDB::bind_method(D_METHOD("get_luts"), &RFXShaderGraph::get_luts);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "luts", PROPERTY_HINT_ARRAY_TYPE, MAKE_RESOURCE_TYPE_HINT("ShaderLUT")), "set_luts", "get_luts");
}

void RFXShaderGraph::set_shader_passes(const TypedArray<ShaderPass> &p_passes) {
	passes.clear();

	for (int i = 0; i < p_passes.size(); i++) {
		Ref<ShaderPass> pass = p_passes[i];

		passes.push_back(pass);
	}
}

TypedArray<ShaderPass> RFXShaderGraph::get_shader_passes() const {
	TypedArray<ShaderPass> res;

	for (int i = 0; i < passes.size(); i++) {
		res.push_back(passes[i]);
	}

	return res;
}

void RFXShaderGraph::set_luts(const TypedArray<ShaderLUT> &p_luts) {
	luts.clear();

	for (int i = 0; i < p_luts.size(); i++) {
		Ref<ShaderLUT> lut = p_luts[i];
		luts.push_back(lut);
	}
}

TypedArray<ShaderLUT> RFXShaderGraph::get_luts() const {
	TypedArray<ShaderLUT> res;

	for (int i = 0; i < luts.size(); i++) {
		res.push_back(luts[i]);
	}

	return res;
}

// region ShaderPass

void ShaderPass::set_shader(const Ref<SlangShader> &p_shader) {
	shader = p_shader;
	emit_changed();
}

Ref<SlangShader> ShaderPass::get_shader() const {
	return shader;
}

void ShaderPass::set_frame_count_mod(uint32_t p_frame_count_mod) {
	frame_count_mod = p_frame_count_mod;
	emit_changed();
}

uint32_t ShaderPass::get_frame_count_mod() const {
	return frame_count_mod;
}

void ShaderPass::set_scale_mode_x(ShaderPassScale p_scale_mode_x) {
	scale_mode_x = p_scale_mode_x;
	notify_property_list_changed();
	emit_changed();
}

ShaderPassScale ShaderPass::get_scale_mode_x() const {
	return scale_mode_x;
}

void ShaderPass::set_scale_x(float p_scale_x) {
	scale_x = p_scale_x;
	emit_changed();
}

float ShaderPass::get_scale_x() const {
	return scale_x;
}

void ShaderPass::set_scale_mode_y(ShaderPassScale p_scale_mode_y) {
	scale_mode_y = p_scale_mode_y;
	notify_property_list_changed();
	emit_changed();
}

ShaderPassScale ShaderPass::get_scale_mode_y() const {
	return scale_mode_y;
}

void ShaderPass::set_scale_y(float p_scale_y) {
	scale_y = p_scale_y;
	emit_changed();
}

float ShaderPass::get_scale_y() const {
	return scale_y;
}

// _bind_methods

void ShaderPass::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_shader", "shader"), &ShaderPass::set_shader);
	ClassDB::bind_method(D_METHOD("get_shader"), &ShaderPass::get_shader);

	ClassDB::bind_method(D_METHOD("set_frame_count_mod", "frame_count_mod"), &ShaderPass::set_frame_count_mod);
	ClassDB::bind_method(D_METHOD("get_frame_count_mod"), &ShaderPass::get_frame_count_mod);

	ClassDB::bind_method(D_METHOD("set_scale_mode_x", "scale_mode_x"), &ShaderPass::set_scale_mode_x);
	ClassDB::bind_method(D_METHOD("get_scale_mode_x"), &ShaderPass::get_scale_mode_x);

	ClassDB::bind_method(D_METHOD("set_scale_x", "scale_x"), &ShaderPass::set_scale_x);
	ClassDB::bind_method(D_METHOD("get_scale_x"), &ShaderPass::get_scale_x);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "shader", PROPERTY_HINT_RESOURCE_TYPE, "SlangShader"), "set_shader", "get_shader");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "frame_count_mod"), "set_frame_count_mod", "get_frame_count_mod");

	ADD_GROUP("Scale", "scale_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "scale_mode_x", PROPERTY_HINT_ENUM, "None,Source,Absolute,Viewport"), "set_scale_mode_x", "get_scale_mode_x");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "scale_x"), "set_scale_x", "get_scale_x");
}

void ShaderPass::_validate_property(PropertyInfo &p_property) const {
	if (p_property.name == "scale_x") {
		switch (scale_mode_x) {
			case ShaderPassScale::NONE:
				p_property.usage = PROPERTY_USAGE_NO_EDITOR;
				break;
			case ShaderPassScale::ABSOLUTE:
				p_property.type = Variant::INT;
				p_property.hint = PROPERTY_HINT_RANGE;
				p_property.hint_string = "100,4096,1,suffix:px";
				break;
			case ShaderPassScale::SOURCE:
				[[fallthrough]];
			case ShaderPassScale::VIEWPORT:
				p_property.hint = PROPERTY_HINT_RANGE;
				p_property.hint_string = "0,5,0.1";
				break;
		}
		return;
	}
}

// endregion
