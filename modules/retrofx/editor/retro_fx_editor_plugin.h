//
// Created by Stuart Carnie on 10/8/2024.
//

#pragma once

#include "resource_importer_slang.h"
#include "resource_importer_slang_preset.h"
#include "retro_fx_inspector_plugin.h"

#include "editor/plugins/editor_plugin.h"

class RetroFXEditorPlugin : public EditorPlugin {
	GDCLASS(RetroFXEditorPlugin, EditorPlugin);

public:
	RetroFXEditorPlugin();

private:
	void _notification(int p_what);

	Ref<ResourceImporterSlang> _slang_importer;
	Ref<ResourceImporterSlangPreset> _slang_preset_importer;
	Ref<RetroFXInspectorPlugin> _inspector_plugin;
};
