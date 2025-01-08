//
// Created by Stuart Carnie on 10/8/2024.
//

#include "retro_fx_editor_plugin.h"

RetroFXEditorPlugin::RetroFXEditorPlugin() {}

void RetroFXEditorPlugin::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_slang_importer.instantiate();
			ResourceFormatImporter::get_singleton()->add_importer(_slang_importer);
			_slang_preset_importer.instantiate();
			ResourceFormatImporter::get_singleton()->add_importer(_slang_preset_importer);
		} break;

		case NOTIFICATION_EXIT_TREE: {
			ResourceFormatImporter::get_singleton()->remove_importer(_slang_preset_importer);
			ResourceFormatImporter::get_singleton()->remove_importer(_slang_importer);
		} break;
	}
}
