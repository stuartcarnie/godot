/**************************************************************************/
/*  retro_fx_rect.cpp                                                     */
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

#include "retro_fx_rect.h"

#include "core/object/callable_mp.h"
#include "core/object/class_db.h"
#include "servers/rendering/renderer_rd/storage_rd/texture_storage.h"
#include "servers/rendering/rendering_server_default.h"

void RetroFXRect::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_update_shader_chain();
			_update_process();
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
			_internal_process();
		} break;
		case NOTIFICATION_DRAW: {
			if (texture.is_null()) {
				return;
			}

			Size2 size;
			Point2 offset;
			Rect2 region;
			bool tile = false;

			switch (stretch_mode) {
				case STRETCH_SCALE: {
					size = get_size();
				} break;
				case STRETCH_TILE: {
					size = get_size();
					tile = true;
				} break;
				case STRETCH_KEEP: {
					size = texture->get_size();
				} break;
				case STRETCH_KEEP_CENTERED: {
					offset = (get_size() - texture->get_size()) / 2;
					size = texture->get_size();
				} break;
				case STRETCH_KEEP_ASPECT_CENTERED:
				case STRETCH_KEEP_ASPECT: {
					size = get_size();
					int tex_width = texture->get_width() * size.height / texture->get_height();
					int tex_height = size.height;

					if (tex_width > size.width) {
						tex_width = size.width;
						tex_height = texture->get_height() * tex_width / texture->get_width();
					}

					if (stretch_mode == STRETCH_KEEP_ASPECT_CENTERED) {
						offset.x += (size.width - tex_width) / 2;
						offset.y += (size.height - tex_height) / 2;
					}

					size.width = tex_width;
					size.height = tex_height;
				} break;
				case STRETCH_KEEP_ASPECT_COVERED: {
					size = get_size();

					Size2 tex_size = texture->get_size();
					Size2 scale_size(size.width / tex_size.width, size.height / tex_size.height);
					float scale = scale_size.width > scale_size.height ? scale_size.width : scale_size.height;
					Size2 scaled_tex_size = tex_size * scale;

					region.position = ((scaled_tex_size - size) / scale).abs() / 2.0f;
					region.size = size / scale;
				} break;
			}

			size.width *= hflip ? -1.0f : 1.0f;
			size.height *= vflip ? -1.0f : 1.0f;

			shader_chain->set_source_rect(Rect2(offset, texture->get_size()), texture->get_size());

			if (region.has_area()) {
				draw_texture_rect_region(output_texture, Rect2(offset, size), region);
			} else {
				draw_texture_rect(output_texture, Rect2(offset, size), tile);
			}
		} break;
		case NOTIFICATION_RESIZED: {
			_update_shader_chain();
			update_minimum_size();
		} break;
	}
}

void RetroFXRect::_internal_process() {
	if (texture.is_null() || output_fb.is_null()) {
		return;
	}
	// TODO(sgc): shader_chain should accept a `Texture2D`, so that it defers this to the
	//  FilterChain, which will have separate versions for RenderingDevice and GLES3
	// If the texture is a ViewportTexture, this ensures that
	auto info = RendererRD::TextureStorage::get_singleton()->canvas_texture_get_info(texture->get_rid(), RSE::CANVAS_ITEM_TEXTURE_FILTER_NEAREST, RSE::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED, false, false);

	shader_chain->render(texture_rid, texture->get_size(), output_fb, get_size());

	// TODO(sgc): should check if Slang shader uses `time` and only call `redraw_request` if it does
	//  Potentially feedback and history might require redraw too.
	RenderingServerDefault::redraw_request();
}

void RetroFXRect::_update_process() {
	set_process_internal(true);
}

Size2 RetroFXRect::get_minimum_size() const {
	if (texture.is_valid()) {
		switch (expand_mode) {
			case EXPAND_KEEP_SIZE: {
				return texture->get_size();
			} break;
			case EXPAND_IGNORE_SIZE: {
				return Size2();
			} break;
			case EXPAND_FIT_WIDTH: {
				return Size2(get_size().y, 0);
			} break;
			case EXPAND_FIT_WIDTH_PROPORTIONAL: {
				real_t ratio = real_t(texture->get_width()) / texture->get_height();
				return Size2(get_size().y * ratio, 0);
			} break;
			case EXPAND_FIT_HEIGHT: {
				return Size2(0, get_size().x);
			} break;
			case EXPAND_FIT_HEIGHT_PROPORTIONAL: {
				real_t ratio = real_t(texture->get_height()) / texture->get_width();
				return Size2(0, get_size().x * ratio);
			} break;
		}
	}
	return Size2();
}

void RetroFXRect::_update_shader_chain() {
	if (shader_chain_error != OK) {
		return;
	}

	RD *rd = RD::get_singleton();

	const Size2 size = get_size();
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
	rd->set_resource_name(output_texture_rid, "RetroFX output");
#endif
	rd->texture_clear(output_texture_rid, Color(0, 0, 0), 0, 1, 0, 1);
	output_texture->set_texture_rd_rid(output_texture_rid);

	output_fb = rd->framebuffer_create({ output_texture_rid });
	rd->framebuffer_set_invalidation_callback(output_fb, [](void *p_ud) {
		RetroFXRect *fx = static_cast<RetroFXRect *>(p_ud);
		fx->output_fb = RID(); }, this);

	queue_redraw();
}

void RetroFXRect::_toggle_pause() {
	set_process_internal(!is_processing_internal());
}

bool RetroFXRect::_get(const StringName &p_name, Variant &r_ret) const {
	Vector<String> parts = String(p_name).split("/", true, 2);
	if (!shader_chain->has_shader_loaded() || parts.size() != 2 || parts[0] != "parameters") {
		return false;
	}

	if (parts[1] == "_pause") {
		r_ret = callable_mp(const_cast<RetroFXRect *>(this), &RetroFXRect::_toggle_pause);
		return true;
	}

	if (double value; shader_chain->get_parameter_value_by_name(parts[1], value)) {
		r_ret = value;
		return true;
	}

	return false;
}

bool RetroFXRect::_set(const StringName &p_name, const Variant &p_value) {
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
		_internal_process();
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

void RetroFXRect::_get_property_list(List<PropertyInfo> *p_list) const {
	if (!shader_chain->has_shader_loaded()) {
		return;
	}

	// Add a pause button
	p_list->push_back(PropertyInfo(Variant::CALLABLE, "parameters/_pause", PROPERTY_HINT_TOOL_BUTTON, "Toggle Pause", PROPERTY_USAGE_EDITOR));

	TypedArray<ShaderParameter> params = shader_chain->get_parameters();
	for (Ref<ShaderParameter> const param : params) {
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

void RetroFXRect::set_intelligent_grouping(bool p_enabled) {
	if (intelligent_grouping == p_enabled) {
		return;
	}
	intelligent_grouping = p_enabled;
	notify_property_list_changed();
}

bool RetroFXRect::_property_can_revert(const StringName &p_name) const {
	Vector<String> parts = String(p_name).split("/", true, 2);

	if (!shader_chain->has_shader_loaded() || parts.size() != 2 || parts[0] != "parameters") {
		return false;
	}

	if (double value; shader_chain->get_default_parameter_value_by_name(parts[1], value)) {
		return true;
	}

	return false;
}

bool RetroFXRect::_property_get_revert(const StringName &p_name, Variant &r_property) const {
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

void RetroFXRect::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_texture", "texture"), &RetroFXRect::set_texture);
	ClassDB::bind_method(D_METHOD("get_texture"), &RetroFXRect::get_texture);
	ClassDB::bind_method(D_METHOD("set_expand_mode", "expand_mode"), &RetroFXRect::set_expand_mode);
	ClassDB::bind_method(D_METHOD("get_expand_mode"), &RetroFXRect::get_expand_mode);
	ClassDB::bind_method(D_METHOD("set_flip_h", "enable"), &RetroFXRect::set_flip_h);
	ClassDB::bind_method(D_METHOD("is_flipped_h"), &RetroFXRect::is_flipped_h);
	ClassDB::bind_method(D_METHOD("set_flip_v", "enable"), &RetroFXRect::set_flip_v);
	ClassDB::bind_method(D_METHOD("is_flipped_v"), &RetroFXRect::is_flipped_v);
	ClassDB::bind_method(D_METHOD("set_stretch_mode", "stretch_mode"), &RetroFXRect::set_stretch_mode);
	ClassDB::bind_method(D_METHOD("get_stretch_mode"), &RetroFXRect::get_stretch_mode);
	ClassDB::bind_method(D_METHOD("set_shader_path", "path"), &RetroFXRect::set_shader_path);
	ClassDB::bind_method(D_METHOD("get_shader_path"), &RetroFXRect::get_shader_path);
	ClassDB::bind_method(D_METHOD("set_intelligent_grouping", "enabled"), &RetroFXRect::set_intelligent_grouping);
	ClassDB::bind_method(D_METHOD("is_intelligent_grouping_enabled"), &RetroFXRect::is_intelligent_grouping_enabled);
	ClassDB::bind_method(D_METHOD("get_shader_chain"), &RetroFXRect::get_shader_chain);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_texture", "get_texture");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "expand_mode", PROPERTY_HINT_ENUM, "Keep Size,Ignore Size,Fit Width,Fit Width Proportional,Fit Height,Fit Height Proportional"), "set_expand_mode", "get_expand_mode");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "stretch_mode", PROPERTY_HINT_ENUM, "Scale,Tile,Keep,Keep Centered,Keep Aspect,Keep Aspect Centered,Keep Aspect Covered"), "set_stretch_mode", "get_stretch_mode");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flip_h"), "set_flip_h", "is_flipped_h");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flip_v"), "set_flip_v", "is_flipped_v");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "shader_path", PROPERTY_HINT_GLOBAL_FILE, "*.slangp"), "set_shader_path", "get_shader_path");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "intelligent_grouping"), "set_intelligent_grouping", "is_intelligent_grouping_enabled");

	BIND_ENUM_CONSTANT(EXPAND_KEEP_SIZE);
	BIND_ENUM_CONSTANT(EXPAND_IGNORE_SIZE);
	BIND_ENUM_CONSTANT(EXPAND_FIT_WIDTH);
	BIND_ENUM_CONSTANT(EXPAND_FIT_WIDTH_PROPORTIONAL);
	BIND_ENUM_CONSTANT(EXPAND_FIT_HEIGHT);
	BIND_ENUM_CONSTANT(EXPAND_FIT_HEIGHT_PROPORTIONAL);

	BIND_ENUM_CONSTANT(STRETCH_SCALE);
	BIND_ENUM_CONSTANT(STRETCH_TILE);
	BIND_ENUM_CONSTANT(STRETCH_KEEP);
	BIND_ENUM_CONSTANT(STRETCH_KEEP_CENTERED);
	BIND_ENUM_CONSTANT(STRETCH_KEEP_ASPECT);
	BIND_ENUM_CONSTANT(STRETCH_KEEP_ASPECT_CENTERED);
	BIND_ENUM_CONSTANT(STRETCH_KEEP_ASPECT_COVERED);
}

void RetroFXRect::_texture_changed() {
	texture_rid = RS::get_singleton()->texture_get_rd_texture(texture->get_rid());
	queue_redraw();
	update_minimum_size();
}

void RetroFXRect::set_texture(const Ref<Texture2D> &p_tex) {
	if (p_tex == texture) {
		return;
	}

	if (texture.is_valid()) {
		texture->disconnect_changed(callable_mp(this, &RetroFXRect::_texture_changed));
	}

	texture = p_tex;

	if (texture.is_valid()) {
		texture->connect_changed(callable_mp(this, &RetroFXRect::_texture_changed));
		texture_rid = RS::get_singleton()->texture_get_rd_texture(texture->get_rid());
	}

	queue_redraw();
	update_minimum_size();
}

Ref<Texture2D> RetroFXRect::get_texture() const {
	return texture;
}

void RetroFXRect::set_expand_mode(ExpandMode p_mode) {
	if (expand_mode == p_mode) {
		return;
	}

	expand_mode = p_mode;
	queue_redraw();
	update_minimum_size();
}

RetroFXRect::ExpandMode RetroFXRect::get_expand_mode() const {
	return expand_mode;
}

void RetroFXRect::set_stretch_mode(StretchMode p_mode) {
	if (stretch_mode == p_mode) {
		return;
	}

	stretch_mode = p_mode;
	queue_redraw();
}

RetroFXRect::StretchMode RetroFXRect::get_stretch_mode() const {
	return stretch_mode;
}

void RetroFXRect::set_flip_h(bool p_flip) {
	if (hflip == p_flip) {
		return;
	}

	hflip = p_flip;
	queue_redraw();
}

bool RetroFXRect::is_flipped_h() const {
	return hflip;
}

void RetroFXRect::set_flip_v(bool p_flip) {
	if (vflip == p_flip) {
		return;
	}

	vflip = p_flip;
	queue_redraw();
}

bool RetroFXRect::is_flipped_v() const {
	return vflip;
}

void RetroFXRect::set_shader_path(const String &p_path) {
	shader_path = p_path;
	shader_chain_error = shader_chain->load_from_file(p_path);

	if (!is_processing_internal()) {
		_internal_process();
	}

	notify_property_list_changed();
	emit_signal(CoreStringName(changed));
}

String RetroFXRect::get_shader_path() const {
	return shader_path;
}

RetroFXRect::RetroFXRect() {
	set_mouse_filter(MOUSE_FILTER_PASS);
	shader_chain.instantiate();
	output_texture.instantiate();

	Vector<RD::AttachmentFormat> attachments;
	{
		RD::AttachmentFormat att;
		att.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
		att.usage_flags = RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | RD::TEXTURE_USAGE_SAMPLING_BIT;
		attachments.push_back(att);
	}
	fb_format = RD::get_singleton()->framebuffer_format_create(attachments);
}

RetroFXRect::~RetroFXRect() {
	RD *rd = RD::get_singleton();

	if (output_texture_rid.is_valid()) {
		rd->free_rid(output_texture_rid);
	}

	if (output_fb.is_valid()) {
		rd->free_rid(output_fb);
	}
}
