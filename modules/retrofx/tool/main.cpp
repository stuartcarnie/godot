//
// Created by Stuart Carnie on 4/8/2024.
//

#include "core/io/file_access.h"
#include "core/register_core_types.h"
#include "drivers/unix/os_unix.h"
#include "modules/slang/ShaderModel.h"
#include "modules/slang/SlangShader.h"
#include "platform/macos/dir_access_macos.h"

class OS_MacOS : public OS_Unix {
protected:
	void initialize_core() override {
		OS_Unix::initialize_core();
		DirAccess::make_default<DirAccessMacOS>(DirAccess::ACCESS_RESOURCES);
		DirAccess::make_default<DirAccessMacOS>(DirAccess::ACCESS_USERDATA);
		DirAccess::make_default<DirAccessMacOS>(DirAccess::ACCESS_FILESYSTEM);

	}

	void initialize() override {
		initialize_core();
	}

	void initialize_joypads() override {

	}

	void finalize() override {

	}

	void set_main_loop(MainLoop *p_main_loop) override {
	}

	void delete_main_loop() override {
	}

	bool _check_internal_feature_support(const String &p_feature) override {
		return false;
	}

	MainLoop *get_main_loop() const override {
		return nullptr;
	}

public:
	OS_MacOS() {};
	void setup() {
		initialize();
	}
	~OS_MacOS() {};
};

Error run() {
	Error err;
	SlangShader shader("/Volumes/Data/projects/libretro/slang-shaders/crt/crt-royale.slangp", err);
	String vert = shader.passes.write[0]._parser.get_vert_source();
	String frag = shader.passes.write[0]._parser.get_frag_source();
	return err;
}

int main(int argc, char **argv) {
	OS_MacOS os;
	os.setup();

	register_core_types();

	run();

	unregister_core_types();

	return 0;
}
