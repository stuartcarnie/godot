//
// Created by Stuart Carnie on 4/8/2024.
//

#ifndef GODOT_CONFIGFILE_H
#define GODOT_CONFIGFILE_H

#include "error.h"

#include <filesystem>
#include <map>
#include <optional>
#include <string>

namespace slang {

using std::u32string, std::map;

namespace fs = std::filesystem;

class ConfigFile {
private:
	map<u32string, u32string> _data;

public:
	error::ErrorOpt read(const fs::path &p_path);

	ConfigFile() {}
	~ConfigFile() {}
};

} //namespace slang

#endif //GODOT_CONFIGFILE_H
