/**************************************************************************/
/*  retro_fx_inspector_plugin.cpp                                         */
/**************************************************************************/

#include "retro_fx_inspector_plugin.h"

#include "core/object/callable_mp.h"
#include "editor/editor_interface.h"
#include "editor/gui/editor_spin_slider.h"
#include "scene/gui/box_container.h"

#include "modules/retrofx/retro_fx_rect.h"
#include "modules/retrofx/shader_chain.h"

// ---------------------------------------------------------------------------
// EditorPropertyShaderColor
// ---------------------------------------------------------------------------

EditorPropertyShaderColor::EditorPropertyShaderColor() {
	picker = memnew(ColorPickerButton);
	picker->set_h_size_flags(SIZE_EXPAND_FILL);
	add_child(picker);
	add_focusable(picker);
	picker->connect("color_changed", callable_mp(this, &EditorPropertyShaderColor::_picker_color_changed));
	// Listen for revert clicks: the base class emits `property_changed` only on revert
	// (our user edits go via `multiple_properties_changed`), so this is a clean trigger
	// to also revert the sibling backing props.
	connect("property_changed", callable_mp(this, &EditorPropertyShaderColor::_on_property_changed));
}

void EditorPropertyShaderColor::_on_property_changed(const StringName &p_prop, const Variant &p_value, const StringName &p_field, bool p_changing) {
	if (backing_props.is_empty() || p_prop != StringName(backing_props[0]) || p_changing) {
		return;
	}
	Vector<String> paths;
	Array values;
	for (int i = 1; i < backing_props.size(); i++) {
		bool valid = false;
		Variant rv = EditorPropertyRevert::get_property_revert_value(get_edited_object(), backing_props[i], &valid);
		if (valid) {
			paths.push_back(backing_props[i]);
			values.push_back(rv);
		}
	}
	if (!paths.is_empty()) {
		emit_signal(SNAME("multiple_properties_changed"), paths, values, false);
	}
}

void EditorPropertyShaderColor::setup(const Vector<String> &p_props, bool p_has_alpha) {
	backing_props = p_props;
	has_alpha = p_has_alpha;
	picker->set_edit_alpha(has_alpha);
}

void EditorPropertyShaderColor::update_property() {
	if (!get_edited_object()) {
		return;
	}
	// Wire `property` to the first backing prop so the base class's revert/status
	// machinery activates. The multi-property registration path skips this.
	if (get_edited_property() == StringName() && !backing_props.is_empty()) {
		set_object_and_property(get_edited_object(), backing_props[0]);
	}
	updating = true;
	Color c;
	c.r = (float)(double)get_edited_object()->get(backing_props[0]);
	c.g = (float)(double)get_edited_object()->get(backing_props[1]);
	c.b = (float)(double)get_edited_object()->get(backing_props[2]);
	c.a = has_alpha && backing_props.size() >= 4 ? (float)(double)get_edited_object()->get(backing_props[3]) : 1.0f;
	picker->set_pick_color(c);
	updating = false;
}

void EditorPropertyShaderColor::_picker_color_changed(const Color &p_color) {
	if (updating) {
		return;
	}
	Vector<String> paths = backing_props;
	Array values;
	values.push_back((double)p_color.r);
	values.push_back((double)p_color.g);
	values.push_back((double)p_color.b);
	if (has_alpha && backing_props.size() >= 4) {
		values.push_back((double)p_color.a);
	} else {
		paths.resize(3);
	}
	emit_signal(SNAME("multiple_properties_changed"), paths, values, false);
}

// ---------------------------------------------------------------------------
// EditorPropertyShaderVector2
// ---------------------------------------------------------------------------

EditorPropertyShaderVector2::EditorPropertyShaderVector2() {
	HBoxContainer *hb = memnew(HBoxContainer);
	hb->set_h_size_flags(SIZE_EXPAND_FILL);
	add_child(hb);

	spin_x = memnew(EditorSpinSlider);
	spin_x->set_label("x");
	spin_x->set_h_size_flags(SIZE_EXPAND_FILL);
	hb->add_child(spin_x);
	add_focusable(spin_x);
	spin_x->connect("value_changed", callable_mp(this, &EditorPropertyShaderVector2::_value_changed).bind(0));

	spin_y = memnew(EditorSpinSlider);
	spin_y->set_label("y");
	spin_y->set_h_size_flags(SIZE_EXPAND_FILL);
	hb->add_child(spin_y);
	add_focusable(spin_y);
	spin_y->connect("value_changed", callable_mp(this, &EditorPropertyShaderVector2::_value_changed).bind(1));
	connect("property_changed", callable_mp(this, &EditorPropertyShaderVector2::_on_property_changed));
}

void EditorPropertyShaderVector2::_on_property_changed(const StringName &p_prop, const Variant &p_value, const StringName &p_field, bool p_changing) {
	if (backing_props.is_empty() || p_prop != StringName(backing_props[0]) || p_changing) {
		return;
	}
	Vector<String> paths;
	Array values;
	for (int i = 1; i < backing_props.size(); i++) {
		bool valid = false;
		Variant rv = EditorPropertyRevert::get_property_revert_value(get_edited_object(), backing_props[i], &valid);
		if (valid) {
			paths.push_back(backing_props[i]);
			values.push_back(rv);
		}
	}
	if (!paths.is_empty()) {
		emit_signal(SNAME("multiple_properties_changed"), paths, values, false);
	}
}

void EditorPropertyShaderVector2::setup(const Vector<String> &p_props, double p_min, double p_max, double p_step) {
	backing_props = p_props;
	for (EditorSpinSlider *s : { spin_x, spin_y }) {
		s->set_min(p_min);
		s->set_max(p_max);
		s->set_step(p_step > 0.0 ? p_step : 0.001);
		s->set_allow_lesser(false);
		s->set_allow_greater(false);
	}
}

void EditorPropertyShaderVector2::update_property() {
	if (!get_edited_object()) {
		return;
	}
	if (get_edited_property() == StringName() && !backing_props.is_empty()) {
		set_object_and_property(get_edited_object(), backing_props[0]);
	}
	updating = true;
	spin_x->set_value((double)get_edited_object()->get(backing_props[0]));
	spin_y->set_value((double)get_edited_object()->get(backing_props[1]));
	updating = false;
}

void EditorPropertyShaderVector2::_value_changed(double p_value, int p_axis) {
	if (updating) {
		return;
	}
	Vector<String> paths = backing_props;
	Array values;
	values.push_back(spin_x->get_value());
	values.push_back(spin_y->get_value());
	emit_signal(SNAME("multiple_properties_changed"), paths, values, false);
}

// ---------------------------------------------------------------------------
// RetroFXInspectorPlugin
// ---------------------------------------------------------------------------

bool RetroFXInspectorPlugin::can_handle(Object *p_object) {
	return Object::cast_to<RetroFXRect>(p_object) != nullptr;
}

// Returns true if `name` ends with `suffix` (case-sensitive). Writes the stem to `r_stem`.
static bool ends_with_suffix(const String &p_name, const String &p_suffix, String &r_stem) {
	if (!p_name.ends_with(p_suffix)) {
		return false;
	}
	r_stem = p_name.substr(0, p_name.length() - p_suffix.length());
	return true;
}

// Longest common prefix of two strings.
static String longest_common_prefix(const String &a, const String &b) {
	const int n = MIN(a.length(), b.length());
	int i = 0;
	while (i < n && a[i] == b[i]) {
		i++;
	}
	return a.substr(0, i);
}

static String derive_label(const Vector<Ref<ShaderParameter>> &p_members, const String &p_fallback_stem) {
	if (p_members.is_empty()) {
		return p_fallback_stem;
	}
	String prefix = p_members[0]->get_desc();
	for (int i = 1; i < p_members.size(); i++) {
		prefix = longest_common_prefix(prefix, p_members[i]->get_desc());
		if (prefix.is_empty()) {
			break;
		}
	}
	prefix = prefix.strip_edges();
	// Trim trailing single-letter or punctuation tokens left behind.
	while (!prefix.is_empty()) {
		const char32_t c = prefix[prefix.length() - 1];
		if (c == ' ' || c == ':' || c == '-' || c == '_' || c == '(') {
			prefix = prefix.substr(0, prefix.length() - 1);
		} else {
			break;
		}
	}
	return prefix.is_empty() ? p_fallback_stem : prefix;
}

void RetroFXInspectorPlugin::_build_groups(RetroFXRect *p_fx) {
	groups_by_member.clear();

	Ref<ShaderChain> chain = p_fx->get_shader_chain();
	if (chain.is_null() || !chain->has_shader_loaded()) {
		return;
	}

	// Build a name -> param map for fast sibling lookup.
	HashMap<String, Ref<ShaderParameter>> by_name;
	TypedArray<ShaderParameter> params = chain->get_parameters();
	for (int i = 0; i < params.size(); i++) {
		Ref<ShaderParameter> p = params[i];
		by_name.insert(p->get_name(), p);
	}

	HashMap<String, bool> consumed;

	auto try_register_group = [&](const String &p_first_member,
									  const Vector<String> &p_required_suffixes_lower,
									  const Vector<String> &p_required_suffixes_upper,
									  bool p_has_alpha_optional, GroupKind p_kind) -> bool {
		// Check both lowercase and uppercase variants.
		for (int variant = 0; variant < 2; variant++) {
			const Vector<String> &suffixes = (variant == 0) ? p_required_suffixes_lower : p_required_suffixes_upper;
			String stem;
			if (!ends_with_suffix(p_first_member, suffixes[0], stem)) {
				continue;
			}
			Vector<Ref<ShaderParameter>> members;
			Vector<String> member_names;
			bool ok = true;
			for (const String &suf : suffixes) {
				const String candidate = stem + suf;
				HashMap<String, Ref<ShaderParameter>>::ConstIterator it = by_name.find(candidate);
				if (!it || consumed.has(candidate)) {
					ok = false;
					break;
				}
				members.push_back(it->value);
				member_names.push_back(candidate);
			}
			if (!ok) {
				continue;
			}

			// Optional alpha for color groups.
			bool with_alpha = false;
			if (p_has_alpha_optional) {
				const Vector<String> alpha_suffixes = (variant == 0)
						? Vector<String>{ "_a", "_alpha" }
						: Vector<String>{ "_A", "_alpha" };
				for (const String &asuf : alpha_suffixes) {
					const String candidate = stem + asuf;
					HashMap<String, Ref<ShaderParameter>>::ConstIterator it = by_name.find(candidate);
					if (it && !consumed.has(candidate)) {
						members.push_back(it->value);
						member_names.push_back(candidate);
						with_alpha = true;
						break;
					}
				}
			}

			GroupInfo info;
			info.kind = with_alpha ? GROUP_COLOR_RGBA : p_kind;
			info.label = derive_label(members, stem);
			info.range_min = members[0]->get_minimum();
			info.range_max = members[0]->get_maximum();
			info.range_step = members[0]->get_step();
			for (const String &n : member_names) {
				info.member_param_names.push_back(n);
				info.member_full_paths.push_back(vformat("parameters/%s", n));
				consumed.insert(n, true);
			}
			for (const String &n : member_names) {
				groups_by_member.insert(n, info);
			}
			return true;
		}
		return false;
	};

	// Walk parameters in original order; first-seen member of any group wins.
	for (int i = 0; i < params.size(); i++) {
		Ref<ShaderParameter> p = params[i];
		const String name = p->get_name();
		if (consumed.has(name)) {
			continue;
		}
		// Color RGB(A): try _r/_g/_b then _R/_G/_B then _red/_green/_blue.
		if (try_register_group(name,
					{ "_r", "_g", "_b" }, { "_R", "_G", "_B" }, true, GROUP_COLOR_RGB)) {
			continue;
		}
		if (try_register_group(name,
					{ "_red", "_green", "_blue" }, { "_red", "_green", "_blue" }, true, GROUP_COLOR_RGB)) {
			continue;
		}
		// Vector2: _x/_y or _X/_Y.
		if (try_register_group(name,
					{ "_x", "_y" }, { "_X", "_Y" }, false, GROUP_VECTOR2)) {
			continue;
		}
	}
}

void RetroFXInspectorPlugin::parse_begin(Object *p_object) {
	groups_by_member.clear();
	RetroFXRect *fx = Object::cast_to<RetroFXRect>(p_object);
	if (!fx || !fx->is_intelligent_grouping_enabled()) {
		return;
	}
	_build_groups(fx);

	// Register tooltip descriptions for every parameter that has one.
	Ref<ShaderChain> chain = fx->get_shader_chain();
	if (chain.is_null() || !chain->has_shader_loaded()) {
		return;
	}
	EditorInspector *insp = EditorInterface::get_singleton()->get_inspector();
	TypedArray<ShaderParameter> params = chain->get_parameters();
	for (int i = 0; i < params.size(); i++) {
		Ref<ShaderParameter> p = params[i];
		if (p->get_desc().is_empty()) {
			continue;
		}
		insp->add_custom_property_description(
				"RetroFXRect",
				vformat("parameters/%s", p->get_name()),
				p->get_desc());
	}
}

bool RetroFXInspectorPlugin::parse_property(Object *p_object, const Variant::Type p_type, const String &p_path,
		const PropertyHint p_hint, const String &p_hint_text,
		const BitField<PropertyUsageFlags> p_usage, const bool p_wide) {
	if (!p_path.begins_with("parameters/")) {
		return false;
	}
	const String param_name = p_path.trim_prefix("parameters/");
	HashMap<String, GroupInfo>::ConstIterator it = groups_by_member.find(param_name);
	if (!it) {
		return false;
	}
	const GroupInfo &g = it->value;

	// Only the first member emits the composite editor; later members are silently consumed.
	if (g.member_param_names[0] != param_name) {
		return true;
	}

	switch (g.kind) {
		case GROUP_COLOR_RGB:
		case GROUP_COLOR_RGBA: {
			EditorPropertyShaderColor *ed = memnew(EditorPropertyShaderColor);
			ed->setup(g.member_full_paths, g.kind == GROUP_COLOR_RGBA);
			add_property_editor_for_multiple_properties(g.label, g.member_full_paths, ed);
			return true;
		}
		case GROUP_VECTOR2: {
			EditorPropertyShaderVector2 *ed = memnew(EditorPropertyShaderVector2);
			ed->setup(g.member_full_paths, g.range_min, g.range_max, g.range_step);
			add_property_editor_for_multiple_properties(g.label, g.member_full_paths, ed);
			return true;
		}
		case GROUP_NONE:
			break;
	}
	return false;
}
