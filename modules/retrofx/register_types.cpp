/**************************************************************************/
/*  register_types.cpp                                                    */
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

#include "register_types.h"

#include "retro_fx_rect.h"
#include "shader_chain.h"
#include "rfx_shader_graph.h"
#include "slang_shader.h"

#include "core/object/class_db.h"
#include "core/os/os.h"
#include "editor/retro_fx_editor_plugin.h"
#include "editor/retro_fx_inspector_plugin.h"

static Ref<ResourceFormatLoaderSlangPreset> resource_loader_slang_preset;

// #define RETROFX_DISABLED

#ifndef RETROFX_DISABLED
void initialize_retrofx_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_CORE) {
		return;
	}

	if (OS::get_singleton()->get_current_rendering_method() == "gl_compatibility") {
		WARN_PRINT_ONCE("RetroFX module is disabled in compatibility rendering mode.");
		return;
	}

	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		GDREGISTER_CLASS(RetroFXRect);

		GDREGISTER_CLASS(SlangShader);
		GDREGISTER_CLASS(RFXShaderGraph);
		GDREGISTER_CLASS(ShaderPass);
		GDREGISTER_CLASS(ShaderLUT);

		GDREGISTER_CLASS(ShaderParameter);
		GDREGISTER_CLASS(ShaderChain);

		resource_loader_slang_preset.instantiate();
		ResourceLoader::add_resource_format_loader(resource_loader_slang_preset);
	}

#if TOOLS_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		GDREGISTER_INTERNAL_CLASS(EditorPropertyShaderColor);
		GDREGISTER_INTERNAL_CLASS(EditorPropertyShaderVector2);
		GDREGISTER_INTERNAL_CLASS(RetroFXInspectorPlugin);
		EditorPlugins::add_by_type<RetroFXEditorPlugin>();
	}
#endif
}

void uninitialize_retrofx_module(ModuleInitializationLevel p_level) {
	if (OS::get_singleton()->get_current_rendering_method() == "gl_compatibility") {
		return;
	}

	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	ResourceLoader::remove_resource_format_loader(resource_loader_slang_preset);
	resource_loader_slang_preset.unref();
}
#else
void initialize_retrofx_module(ModuleInitializationLevel p_level) {}
void uninitialize_retrofx_module(ModuleInitializationLevel p_level) {}
#endif
