/**************************************************************************/
/*  retro_fx_container.h                                                  */
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

#pragma once

#include "shader_chain.h"

#include "scene/gui/subviewport_container.h"
#include "scene/resources/texture_rd.h"

class SubViewport;

// A SubViewportContainer that draws its first SubViewport child through a
// RetroFX shader chain. The SubViewport keeps its own size (for example 320x200)
// and the chain renders it into an output texture the size of this container.
class RetroFXContainer : public SubViewportContainer {
	GDCLASS(RetroFXContainer, SubViewportContainer);

	String shader_path;
	bool intelligent_grouping = true;

	Ref<ShaderChain> shader_chain;
	Error shader_chain_error = OK;
	bool paused = false;
	bool presenting = false; // A redraw was requested to show the last render; skip re-rendering that frame.

	SubViewport *source = nullptr;
	RID source_texture_rid;

	RID output_texture_rid;
	Ref<Texture2DRD> output_texture;
	RID output_fb;

	void _set_source(SubViewport *p_source);
	void _source_texture_changed();
	void _update_output();
	void _render();
	void _toggle_pause();
	void _frame_post_draw();

protected:
	void _notification(int p_what);
	void _validate_property(PropertyInfo &p_property) const;

	virtual void add_child_notify(Node *p_child) override;
	virtual void remove_child_notify(Node *p_child) override;

	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;
	bool _property_can_revert(const StringName &p_name) const;
	bool _property_get_revert(const StringName &p_name, Variant &r_property) const;

	static void _bind_methods();

public:
	virtual Transform2D get_child_viewport_transform() const override;

	void set_shader_path(const String &p_path);
	String get_shader_path() const;

	void set_intelligent_grouping(bool p_enabled);
	bool is_intelligent_grouping_enabled() const { return intelligent_grouping; }

	void set_filter_enabled(bool p_enabled);
	bool is_filter_enabled() const;

	Ref<ShaderChain> get_shader_chain() const { return shader_chain; }

	PackedStringArray get_configuration_warnings() const override;

	RetroFXContainer();
	~RetroFXContainer() override;
};
