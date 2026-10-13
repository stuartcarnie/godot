/**************************************************************************/
/*  retro_fx_container.cpp                                                */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "retro_fx_container.h"

#include "core/object/callable_mp.h"
#include "core/object/class_db.h"
#include "scene/main/viewport.h"
#include "servers/rendering/rendering_server_default.h"

void RetroFXContainer::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_update_output();
			set_process_internal(true);
		} break;

		case NOTIFICATION_INTERNAL_PROCESS: {
			_render();
		} break;

		case NOTIFICATION_DRAW: {
			if (source == nullptr || output_texture_rid.is_null()) {
				return; // Keep the base class drawing of the raw viewport texture.
			}
			// SubViewportContainer has already queued the raw viewport texture. Drop it
			// and draw the processed output over the whole container instead.
			RS::get_singleton()->canvas_item_clear(get_canvas_item());
			draw_texture_rect(output_texture, Rect2(Point2(), get_size()));
		} break;

		case NOTIFICATION_RESIZED: {
			_update_output();
		} break;
	}
}

void RetroFXContainer::_validate_property(PropertyInfo &p_property) const {
	// The child SubViewport keeps its own size; the shader chain scales it to the container.
	if (p_property.name == "stretch" || p_property.name == "stretch_shrink") {
		p_property.usage = PROPERTY_USAGE_NONE;
	}
}

void RetroFXContainer::add_child_notify(Node *p_child) {
	SubViewportContainer::add_child_notify(p_child);

	SubViewport *vp = Object::cast_to<SubViewport>(p_child);
	if (vp != nullptr && source == nullptr) {
		_set_source(vp);
	}
	update_configuration_warnings();
}

void RetroFXContainer::remove_child_notify(Node *p_child) {
	SubViewportContainer::remove_child_notify(p_child);

	if (p_child == source) {
		_set_source(nullptr);
	}
	update_configuration_warnings();
}

void RetroFXContainer::_set_source(SubViewport *p_source) {
	if (source == p_source) {
		return;
	}

	if (source != nullptr) {
		source->get_texture()->disconnect_changed(callable_mp(this, &RetroFXContainer::_source_texture_changed));
	}

	source = p_source;
	source_texture_rid = RID();

	if (source != nullptr) {
		source->get_texture()->connect_changed(callable_mp(this, &RetroFXContainer::_source_texture_changed));
		_source_texture_changed();
	}

	queue_redraw();
}

void RetroFXContainer::_source_texture_changed() {
	source_texture_rid = RS::get_singleton()->texture_get_rd_texture(source->get_texture()->get_rid());
}

void RetroFXContainer::_update_output() {
	RD *rd = RD::get_singleton();
	if (rd == nullptr) {
		return; // No RenderingDevice, such as when running headless or generating documentation.
	}

	const Size2i size = get_size();
	if (size.width < 1 || size.height < 1) {
		return;
	}

	shader_chain->set_drawable_size(size);

	if (output_fb.is_valid()) {
		rd->free_rid(output_fb);
	}

	if (rd->texture_is_valid(output_texture_rid)) {
		output_texture->set_texture_rd_rid(RID());
		rd->free_rid(output_texture_rid);
		output_texture_rid = RID();
	}

	RD::TextureFormat tf;
	tf.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
	tf.width = size.width;
	tf.height = size.height;
	tf.usage_bits = RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_COPY_TO_BIT;
	tf.texture_type = RD::TEXTURE_TYPE_2D;
	output_texture_rid = rd->texture_create(tf, RD::TextureView());
#if DEV_ENABLED
	rd->set_resource_name(output_texture_rid, "RetroFXContainer output");
#endif
	rd->texture_clear(output_texture_rid, Color(0, 0, 0), 0, 1, 0, 1);
	output_texture->set_texture_rd_rid(output_texture_rid);

	output_fb = rd->framebuffer_create({ output_texture_rid });
	rd->framebuffer_set_invalidation_callback(output_fb, [](void *p_ud) {
		RetroFXContainer *fx = static_cast<RetroFXContainer *>(p_ud);
		fx->output_fb = RID(); }, this);

	queue_redraw();
}

void RetroFXContainer::_render() {
	if (source == nullptr || output_fb.is_null()) {
		return;
	}

	RD *rd = RD::get_singleton();
	if (!rd->texture_is_valid(source_texture_rid)) {
		// The render target texture is recreated when the viewport is resized or its flags change.
		_source_texture_changed();
		if (source_texture_rid.is_null()) {
			return;
		}
	}

	const Size2i source_size = source->get_size();
	shader_chain->set_source_rect(Rect2i(Point2i(), source_size), source_size);
	shader_chain->render(source_texture_rid, source_size, output_fb, get_size());

	// TODO(sgc): should check if Slang shader uses `time` and only call `redraw_request` if it does
	//  Potentially feedback and history might require redraw too.
	presenting = true;
	RenderingServerDefault::redraw_request();
}

void RetroFXContainer::_frame_post_draw() {
	if (presenting) {
		// This frame was requested by _render() to show the output.
		presenting = false;
		return;
	}
	// Something else redrew the editor, so the source viewport may have changed.
	_render();
}

Transform2D RetroFXContainer::get_child_viewport_transform() const {
	const Rect2 bounds = shader_chain->get_output_bounds();
	if (source == nullptr || is_stretch_enabled() || !bounds.has_area()) {
		// With stretch enabled the viewport follows the container size and the base class handles it.
		return SubViewportContainer::get_child_viewport_transform();
	}
	// Viewport coordinates to container coordinates: scale to the letterboxed output, then offset.
	Transform2D xform;
	xform.scale(bounds.size / Size2(source->get_size()));
	xform.set_origin(bounds.position);
	return xform;
}

// Paused: no continuous redraws, but re-render after any frame drawn for other reasons so
// edits to the source viewport still show.
void RetroFXContainer::_toggle_pause() {
	paused = !paused;
	shader_chain->set_paused(paused);
	set_process_internal(!paused);
	presenting = false;
	if (paused) {
		RS::get_singleton()->connect("frame_post_draw", callable_mp(this, &RetroFXContainer::_frame_post_draw));
	} else {
		RS::get_singleton()->disconnect("frame_post_draw", callable_mp(this, &RetroFXContainer::_frame_post_draw));
	}
}

void RetroFXContainer::set_shader_path(const String &p_path) {
	shader_path = p_path;
	shader_chain_error = shader_chain->load_from_file(p_path);

	if (!is_processing_internal()) {
		_render();
	}

	notify_property_list_changed();
	update_configuration_warnings();
}

String RetroFXContainer::get_shader_path() const {
	return shader_path;
}

void RetroFXContainer::set_filter_enabled(bool p_enabled) {
	if (shader_chain->is_enabled() == p_enabled) {
		return;
	}
	shader_chain->set_enabled(p_enabled);

	if (!is_processing_internal()) {
		_render();
	}
}

bool RetroFXContainer::is_filter_enabled() const {
	return shader_chain->is_enabled();
}

void RetroFXContainer::set_intelligent_grouping(bool p_enabled) {
	if (intelligent_grouping == p_enabled) {
		return;
	}
	intelligent_grouping = p_enabled;
	notify_property_list_changed();
}

PackedStringArray RetroFXContainer::get_configuration_warnings() const {
	PackedStringArray warnings = SubViewportContainer::get_configuration_warnings();

	int viewport_count = 0;
	for (int i = 0; i < get_child_count(); i++) {
		if (Object::cast_to<SubViewport>(get_child(i))) {
			viewport_count++;
		}
	}
	if (viewport_count > 1) {
		warnings.push_back(RTR("Only the first SubViewport child is drawn through the shader chain."));
	}

	if (shader_chain_error != OK) {
		warnings.push_back(vformat(RTR("Failed to load shader preset \"%s\"."), shader_path));
	}

	return warnings;
}

bool RetroFXContainer::_get(const StringName &p_name, Variant &r_ret) const {
	Vector<String> parts = String(p_name).split("/", true, 2);
	if (!shader_chain->has_shader_loaded() || parts.size() != 2 || parts[0] != "parameters") {
		return false;
	}

	if (parts[1] == "_pause") {
		r_ret = callable_mp(const_cast<RetroFXContainer *>(this), &RetroFXContainer::_toggle_pause);
		return true;
	}

	if (double value; shader_chain->get_parameter_value_by_name(parts[1], value)) {
		r_ret = value;
		return true;
	}

	return false;
}

bool RetroFXContainer::_set(const StringName &p_name, const Variant &p_value) {
	Vector<String> parts = String(p_name).split("/", true, 2);

	if (!shader_chain->has_shader_loaded() || parts.size() != 2 || parts[0] != "parameters") {
		return false;
	}

	// For boolean values, convert to 0 or 1
	double value = p_value;
	if (Math::is_zero_approx(value)) {
		value = 0.0;
	} else if (Math::is_equal_approx(value, 1.0)) {
		value = 1.0;
	}

	shader_chain->set_parameter_value_by_name(parts[1], value);

	if (!is_processing_internal()) {
		_render();
	}

	return true;
}

// Detect libretro slang-shader UI sentinels:
//   *_nonono / *_dummy_header / *_dummy_footer / *_unused# / *_comment_header
//   → desc holds a section title (often wrapped in === or ---), used as a section header.
static bool is_section_header_sentinel(const Ref<ShaderParameter> &p_param) {
	const String name = p_param->get_name().to_lower();
	if (name.ends_with("_nonono") || name.contains("nonono")) {
		return true;
	}
	if (name.contains("dummy_header") || name.contains("dummy_footer") || name.contains("comment_header")) {
		return true;
	}
	if (name.contains("_unused")) {
		return true;
	}
	return false;
}

// Detect spacer/divider rows: name like *_space# / *-row#, or desc that is purely whitespace
// or made of repeated punctuation.
static bool is_spacer_sentinel(const Ref<ShaderParameter> &p_param) {
	const String name = p_param->get_name().to_lower();
	if (name.contains("_space") || name.contains("-row")) {
		return true;
	}
	const String desc = p_param->get_desc().strip_edges();
	if (desc.is_empty()) {
		return true;
	}
	bool only_punct = true;
	for (int i = 0; i < desc.length(); i++) {
		const char32_t c = desc[i];
		if (c != '-' && c != '=' && c != '#' && c != '>' && c != '<' && c != '_' && c != '*') {
			only_punct = false;
			break;
		}
	}
	return only_punct && desc.length() >= 3;
}

// Strip decorative wrapping like "==== Title ====" or "--- Title ---" down to "Title".
static String clean_section_title(const String &p_desc) {
	String s = p_desc.strip_edges();
	while (!s.is_empty() && (s[0] == '=' || s[0] == '-' || s[0] == '#' || s[0] == '>')) {
		s = s.substr(1);
	}
	while (!s.is_empty()) {
		const char32_t c = s[s.length() - 1];
		if (c != '=' && c != '-' && c != '#' && c != '<' && c != ':') {
			break;
		}
		s = s.substr(0, s.length() - 1);
	}
	return s.strip_edges();
}

void RetroFXContainer::_get_property_list(List<PropertyInfo> *p_list) const {
	if (!shader_chain->has_shader_loaded()) {
		return;
	}

	// Add a pause button
	p_list->push_back(PropertyInfo(Variant::CALLABLE, "parameters/_pause", PROPERTY_HINT_TOOL_BUTTON, "Toggle Pause", PROPERTY_USAGE_EDITOR));

	TypedArray<ShaderParameter> params = shader_chain->get_parameters();
	for (const Ref<ShaderParameter> param : params) {
		if (intelligent_grouping) {
			if (is_spacer_sentinel(param)) {
				continue;
			}
			if (is_section_header_sentinel(param)) {
				// Render as a native collapsible group covering subsequent `parameters/*` rows.
				PropertyInfo pi(Variant::NIL, clean_section_title(param->get_desc()), PROPERTY_HINT_NONE, "parameters/", PROPERTY_USAGE_GROUP);
				p_list->push_back(pi);
				continue;
			}
		}

		if (param->is_boolean()) {
			p_list->push_back(PropertyInfo(Variant::BOOL, vformat("parameters/%s", param->get_name()), PROPERTY_HINT_NONE));
		} else {
			Variant::Type type;
			String hint_string;
			const PropertyHint hint = Math::is_equal_approx(param->get_minimum(), param->get_maximum()) ? PROPERTY_HINT_NONE : PROPERTY_HINT_RANGE;
			const bool has_step = !Math::is_zero_approx(param->get_step());

			if (param->is_integer()) {
				type = Variant::INT;
				if (hint == PROPERTY_HINT_RANGE) {
					hint_string = has_step ? vformat("%d,%d,%d", int(param->get_minimum()), int(param->get_maximum()), int(param->get_step())) : vformat("%d,%d", int(param->get_minimum()), int(param->get_maximum()));
				}
			} else {
				type = Variant::FLOAT;
				if (hint == PROPERTY_HINT_RANGE) {
					hint_string = has_step ? vformat("%.2f,%.2f,%.2f", param->get_minimum(), param->get_maximum(), param->get_step()) : vformat("%.2f,%.2f", param->get_minimum(), param->get_maximum());
				}
			}
			p_list->push_back(PropertyInfo(type, vformat("parameters/%s", param->get_name()), hint, hint_string));
		}
	}
}

bool RetroFXContainer::_property_can_revert(const StringName &p_name) const {
	Vector<String> parts = String(p_name).split("/", true, 2);

	if (!shader_chain->has_shader_loaded() || parts.size() != 2 || parts[0] != "parameters") {
		return false;
	}

	if (double value; shader_chain->get_default_parameter_value_by_name(parts[1], value)) {
		return true;
	}

	return false;
}

bool RetroFXContainer::_property_get_revert(const StringName &p_name, Variant &r_property) const {
	Vector<String> parts = String(p_name).split("/", true, 2);

	if (!shader_chain->has_shader_loaded() || parts.size() != 2 || parts[0] != "parameters") {
		return false;
	}

	if (double value; shader_chain->get_default_parameter_value_by_name(parts[1], value)) {
		r_property = value;
		return true;
	}

	return false;
}

void RetroFXContainer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_shader_path", "path"), &RetroFXContainer::set_shader_path);
	ClassDB::bind_method(D_METHOD("get_shader_path"), &RetroFXContainer::get_shader_path);
	ClassDB::bind_method(D_METHOD("set_filter_enabled", "enabled"), &RetroFXContainer::set_filter_enabled);
	ClassDB::bind_method(D_METHOD("is_filter_enabled"), &RetroFXContainer::is_filter_enabled);
	ClassDB::bind_method(D_METHOD("set_intelligent_grouping", "enabled"), &RetroFXContainer::set_intelligent_grouping);
	ClassDB::bind_method(D_METHOD("is_intelligent_grouping_enabled"), &RetroFXContainer::is_intelligent_grouping_enabled);
	ClassDB::bind_method(D_METHOD("get_shader_chain"), &RetroFXContainer::get_shader_chain);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "shader_path", PROPERTY_HINT_GLOBAL_FILE, "*.slangp"), "set_shader_path", "get_shader_path");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "filter_enabled"), "set_filter_enabled", "is_filter_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "intelligent_grouping"), "set_intelligent_grouping", "is_intelligent_grouping_enabled");
}

RetroFXContainer::RetroFXContainer() {
	shader_chain.instantiate();
	output_texture.instantiate();
}

RetroFXContainer::~RetroFXContainer() {
	if (paused) {
		RS::get_singleton()->disconnect("frame_post_draw", callable_mp(this, &RetroFXContainer::_frame_post_draw));
	}

	RD *rd = RD::get_singleton();
	if (rd == nullptr) {
		return;
	}

	if (output_texture_rid.is_valid()) {
		rd->free_rid(output_texture_rid);
	}

	if (output_fb.is_valid()) {
		rd->free_rid(output_fb);
	}
}
