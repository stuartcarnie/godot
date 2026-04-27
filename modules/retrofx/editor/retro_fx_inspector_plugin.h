/**************************************************************************/
/*  retro_fx_inspector_plugin.h                                           */
/**************************************************************************/

#pragma once

#include "core/templates/hash_map.h"
#include "editor/inspector/editor_inspector.h"
#include "editor/inspector/editor_properties.h"
#include "scene/gui/color_picker.h"

class EditorPropertyShaderColor : public EditorProperty {
	GDCLASS(EditorPropertyShaderColor, EditorProperty);

	ColorPickerButton *picker = nullptr;
	Vector<String> backing_props;
	bool has_alpha = false;
	bool updating = false;

	void _picker_color_changed(const Color &p_color);
	void _on_property_changed(const StringName &p_prop, const Variant &p_value, const StringName &p_field, bool p_changing);

public:
	virtual void update_property() override;
	void setup(const Vector<String> &p_props, bool p_has_alpha);

	EditorPropertyShaderColor();
};

class EditorPropertyShaderVector2 : public EditorProperty {
	GDCLASS(EditorPropertyShaderVector2, EditorProperty);

	EditorSpinSlider *spin_x = nullptr;
	EditorSpinSlider *spin_y = nullptr;
	Vector<String> backing_props;
	bool updating = false;

	void _value_changed(double p_value, int p_axis);
	void _on_property_changed(const StringName &p_prop, const Variant &p_value, const StringName &p_field, bool p_changing);

public:
	virtual void update_property() override;
	void setup(const Vector<String> &p_props, double p_min, double p_max, double p_step);

	EditorPropertyShaderVector2();
};

class RetroFXInspectorPlugin : public EditorInspectorPlugin {
	GDCLASS(RetroFXInspectorPlugin, EditorInspectorPlugin);

	enum GroupKind {
		GROUP_NONE,
		GROUP_COLOR_RGB,
		GROUP_COLOR_RGBA,
		GROUP_VECTOR2,
	};

	struct GroupInfo {
		GroupKind kind = GROUP_NONE;
		Vector<String> member_param_names;
		Vector<String> member_full_paths;
		String label;
		double range_min = 0.0;
		double range_max = 1.0;
		double range_step = 0.0;
	};

	HashMap<String, GroupInfo> groups_by_member;

	void _build_groups(class RetroFXRect *p_fx);

public:
	virtual bool can_handle(Object *p_object) override;
	virtual void parse_begin(Object *p_object) override;
	virtual bool parse_property(Object *p_object, const Variant::Type p_type, const String &p_path,
			const PropertyHint p_hint, const String &p_hint_text,
			const BitField<PropertyUsageFlags> p_usage, const bool p_wide) override;
};
