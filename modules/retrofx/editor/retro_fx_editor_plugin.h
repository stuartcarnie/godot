//
// Created by Stuart Carnie on 10/8/2024.
//

#ifndef GODOT_RETRO_FX_EDITOR_PLUGIN_H
#define GODOT_RETRO_FX_EDITOR_PLUGIN_H

#include "editor/plugins/editor_plugin.h"
#include "resource_importer_slang.h"
#include "resource_importer_slang_preset.h"

class RetroFXEditorPlugin : public EditorPlugin {
	GDCLASS(RetroFXEditorPlugin, EditorPlugin);
public:
	RetroFXEditorPlugin();

private:
	void _notification(int p_what);

	Ref<ResourceImporterSlang> _slang_importer;
	Ref<ResourceImporterSlangPreset> _slang_preset_importer;
};

#endif //GODOT_RETRO_FX_EDITOR_PLUGIN_H
