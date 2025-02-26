//
// Created by Stuart Carnie on 1/1/2025.
//

#pragma once

#include "core/io/file_access.h"
#include "core/templates/vector.h"
#include <compiled.h>

using namespace slang;

class ShaderContainer {
public:
	virtual const compiled::Shader &get_shader() const = 0;
	virtual Vector<uint8_t> get_lut_by_name(const std::string &p_name) const = 0;

	ShaderContainer() = default;
	virtual ~ShaderContainer() = default;
};

class FileShaderContainer : public ShaderContainer {
	compiled::Shader shader;

	FileShaderContainer(compiled::Shader &&p_shader) :
			shader(std::move(p_shader)) {}

public:
	const compiled::Shader &get_shader() const override {
		return shader;
	}

	Vector<uint8_t> get_lut_by_name(const std::string &p_name) const override {
		// find first LUT with matching name
		for (auto &lut : shader.luts) {
			if (lut.name == p_name) {
				String path = lut.url.c_str();
				Ref<FileAccess> f = FileAccess::open(path, FileAccess::READ);
				return f->get_buffer(f->get_length());
			}
		}

		return Vector<uint8_t>();
	}

	virtual ~FileShaderContainer() override;

	static std::optional<FileShaderContainer> create(const String &p_path);
};
