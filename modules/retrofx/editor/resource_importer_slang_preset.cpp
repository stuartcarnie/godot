//
// Created by Stuart Carnie on 11/8/2024.
//

#include "resource_importer_slang_preset.h"
#include "core/io/file_access.h"

String ResourceImporterSlangPreset::get_importer_name() const {
	return "slang_shader_preset";
}

String ResourceImporterSlangPreset::get_visible_name() const {
	return "SlangShaderPreset";
}

void ResourceImporterSlangPreset::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("slangp");
}

String ResourceImporterSlangPreset::get_save_extension() const {
	return "slangpp";
}

String ResourceImporterSlangPreset::get_resource_type() const {
	return "SlangShader";
}

int ResourceImporterSlangPreset::get_format_version() const {
	return 1;
}

int ResourceImporterSlangPreset::get_preset_count() const {
	return 0;
}

String ResourceImporterSlangPreset::get_preset_name(int p_idx) const {
	return "";
}

void ResourceImporterSlangPreset::get_import_options(const String &p_path, List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "generate_tangents"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::VECTOR3, "scale_mesh"), Vector3(1, 1, 1)));
	r_options->push_back(ImportOption(PropertyInfo(Variant::VECTOR3, "offset_mesh"), Vector3(0, 0, 0)));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "optimize_mesh"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "force_disable_mesh_compression"), false));
}

bool ResourceImporterSlangPreset::get_option_visibility(const String &p_path, const String &p_option, const HashMap<StringName, Variant> &p_options) const {
	return true;
}

Error ResourceImporterSlangPreset::import(ResourceUID::ID p_source_id, const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata) {
	String bd = p_source_file.get_base_dir();
	Ref<FileAccess> f = FileAccess::open(p_source_file, FileAccess::READ);
	if (f.is_null()) {
		return f->get_error();
	}

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

ResourceImporterSlangPreset::ResourceImporterSlangPreset() {
}

Ref<Resource> ResourceFormatLoaderSlangPreset::load(const String &p_path, const String &p_original_path, Error *r_error, bool p_use_sub_threads, float *r_progress, ResourceFormatLoader::CacheMode p_cache_mode)  {
	return Ref<Resource>();
}

void ResourceFormatLoaderSlangPreset::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("slangp");
}

bool ResourceFormatLoaderSlangPreset::handles_type(const String &p_type) const {
	return p_type == "SlangShader";
}

String ResourceFormatLoaderSlangPreset::get_resource_type(const String &p_path) const {
	return "SlangShader";
}


void ResourceFormatLoaderSlangPreset::get_dependencies(const String &p_path, List<String> *p_dependencies, bool p_add_types) {
	print_line(p_path);
}
