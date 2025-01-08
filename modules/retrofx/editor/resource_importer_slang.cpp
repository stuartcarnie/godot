//
// Created by Stuart Carnie on 10/8/2024.
//

#include "resource_importer_slang.h"

String ResourceImporterSlang::get_importer_name() const {
	return "slang_shader";
}

String ResourceImporterSlang::get_visible_name() const {
	return "SlangShader";
}

void ResourceImporterSlang::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("slang");
}

String ResourceImporterSlang::get_save_extension() const {
	return "slangg";
}

String ResourceImporterSlang::get_resource_type() const {
	return "SlangShader";
}

int ResourceImporterSlang::get_format_version() const {
	return 1;
}

int ResourceImporterSlang::get_preset_count() const {
	return 0;
}

String ResourceImporterSlang::get_preset_name(int p_idx) const {
	return "";
}

void ResourceImporterSlang::get_import_options(const String &p_path, List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "generate_tangents"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::VECTOR3, "scale_mesh"), Vector3(1, 1, 1)));
	r_options->push_back(ImportOption(PropertyInfo(Variant::VECTOR3, "offset_mesh"), Vector3(0, 0, 0)));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "optimize_mesh"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "force_disable_mesh_compression"), false));
}

bool ResourceImporterSlang::get_option_visibility(const String &p_path, const String &p_option, const HashMap<StringName, Variant> &p_options) const {
	return true;
}

Error ResourceImporterSlang::import(ResourceUID::ID p_source_id, const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata) {
//	List<Ref<ImporterMesh>> meshes;
//
//	Error err = _parse_obj(p_source_file, meshes, true, p_options["generate_tangents"], p_options["optimize_mesh"], p_options["scale_mesh"], p_options["offset_mesh"], p_options["force_disable_mesh_compression"], nullptr);
//
//	ERR_FAIL_COND_V(err != OK, err);
//	ERR_FAIL_COND_V(meshes.size() != 1, ERR_BUG);
//
//	String save_path = p_save_path + ".mesh";
//
//	err = ResourceSaver::save(meshes.front()->get()->get_mesh(), save_path);
//
//	ERR_FAIL_COND_V_MSG(err != OK, err, "Cannot save Mesh to file '" + save_path + "'.");
//
//	r_gen_files->push_back(save_path);

	return FAILED;
}

ResourceImporterSlang::ResourceImporterSlang() {
}
