//
// Created by Stuart Carnie on 1/1/2025.
//

#include "shader_chain.h"

#include "core/object/class_db.h"

void ShaderParameter::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_index"), &ShaderParameter::get_index);
	ClassDB::bind_method(D_METHOD("get_name"), &ShaderParameter::get_name);
	ClassDB::bind_method(D_METHOD("get_desc"), &ShaderParameter::get_desc);
	ClassDB::bind_method(D_METHOD("get_initial"), &ShaderParameter::get_initial);
	ClassDB::bind_method(D_METHOD("get_minimum"), &ShaderParameter::get_minimum);
	ClassDB::bind_method(D_METHOD("get_maximum"), &ShaderParameter::get_maximum);
	ClassDB::bind_method(D_METHOD("get_step"), &ShaderParameter::get_step);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "index"), "", "get_index");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "name"), "", "get_name");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "desc"), "", "get_desc");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "initial"), "", "get_initial");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "minimum"), "", "get_minimum");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "maximum"), "", "get_maximum");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "step"), "", "get_step");
}

void ShaderChain::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load_from_file", "path"), &ShaderChain::load_from_file);
	ClassDB::bind_method(D_METHOD("set_source_rect", "rect", "aspect"), &ShaderChain::set_source_rect);
	ClassDB::bind_method(D_METHOD("get_source_rect"), &ShaderChain::get_source_rect);
	ClassDB::bind_method(D_METHOD("set_drawable_size", "size"), &ShaderChain::set_drawable_size);
	ClassDB::bind_method(D_METHOD("get_drawable_size"), &ShaderChain::get_drawable_size);
	ClassDB::bind_method(D_METHOD("get_output_bounds"), &ShaderChain::get_output_bounds);
	ClassDB::bind_method(D_METHOD("get_parameters"), &ShaderChain::get_parameters);
	ClassDB::bind_method(D_METHOD("set_parameter_value_by_index", "index", "value"), &ShaderChain::set_parameter_value_by_index);
	ClassDB::bind_method(D_METHOD("set_parameter_value_by_name", "name", "value"), &ShaderChain::set_parameter_value_by_name);
	ClassDB::bind_method(D_METHOD("render", "source", "source_size", "target", "target_size"), &ShaderChain::render);
	ClassDB::bind_method(D_METHOD("get_has_shader_loaded"), &ShaderChain::has_shader_loaded);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "drawable_size"), "set_drawable_size", "get_drawable_size");
	ADD_PROPERTY(PropertyInfo(Variant::RECT2I, "output_bounds"), "", "get_output_bounds");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "has_shader_loaded"), "", "get_has_shader_loaded");
}

Error ShaderChain::load_from_file(const String &p_path) {
	std::optional<FileShaderContainer> res = FileShaderContainer::create(p_path);
	if (!res.has_value()) {
		return ERR_FILE_CANT_OPEN;
	}

	FileShaderContainer container = std::move(res.value());
	std::vector<slang::compiled::Parameter> const &params = container.get_shader().parameters;
	parameter_name_to_index.reserve(params.size());
	parameters.resize(params.size());
	for (const compiled::Parameter &param : params) {
		Ref<ShaderParameter> p;
		p.instantiate(param);
		parameters[param.index] = p;
		parameter_name_to_index.insert(p->get_name(), param.index);
	}

	return filter_chain->set_compiled_shader(container);
}

TypedArray<ShaderParameter> ShaderChain::get_parameters() const {
	return parameters;
}

bool ShaderChain::get_default_parameter_value_by_index(uint32_t p_index, double &r_value) const {
	if (p_index >= (uint32_t)parameters.size()) {
		return false;
	}

	if (Ref<ShaderParameter> p = parameters[p_index]; p.is_valid()) {
		r_value = p->get_initial();
		return true;
	}

	return false;
}

bool ShaderChain::get_default_parameter_value_by_name(const String &p_name, double &r_value) const {
	if (const uint32_t *index = parameter_name_to_index.getptr(p_name)) {
		return get_default_parameter_value_by_index(*index, r_value);
	}

	return false;
}

ShaderChain::ShaderChain() {
	filter_chain = memnew(FilterChain);
}

ShaderChain::~ShaderChain() {
	memdelete(filter_chain);
}
