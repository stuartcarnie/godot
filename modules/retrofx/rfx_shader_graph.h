//
// Created by Stuart Carnie on 8/8/2024.
//

#pragma once

#include "slang_shader.h"

#include "core/io/resource.h"
#include "scene/resources/texture.h"

enum ShaderPassScale {
	NONE,
	SOURCE,
	ABSOLUTE,
	VIEWPORT,
};

class ShaderLUT : public Resource {
	GDCLASS(ShaderLUT, Resource);

public:
private:
	String name;
	Ref<Texture2D> texture;
};

class ShaderPass : public Resource {
	GDCLASS(ShaderPass, Resource);

public:
	void set_shader(const Ref<SlangShader> &p_shader);
	Ref<SlangShader> get_shader() const;

	void set_frame_count_mod(uint32_t p_frame_count_mod);
	uint32_t get_frame_count_mod() const;

	void set_scale_mode_x(ShaderPassScale p_scale_mode_x);
	ShaderPassScale get_scale_mode_x() const;

	void set_scale_x(float p_scale_x);
	float get_scale_x() const;

	void set_scale_mode_y(ShaderPassScale p_scale_mode_y);
	ShaderPassScale get_scale_mode_y() const;

	void set_scale_y(float p_scale_y);
	float get_scale_y() const;

protected:
	void _validate_property(PropertyInfo &p_property) const;

private:
	static void _bind_methods();

	Ref<SlangShader> shader;
	uint32_t frame_count_mod;
	ShaderPassScale scale_mode_x;
	float scale_x;
	ShaderPassScale scale_mode_y;
	float scale_y;
};

class RFXShaderGraph : public Resource {
	GDCLASS(RFXShaderGraph, Resource);

public:
#ifdef TOOLS_ENABLED
	void get_configuration_warnings(PackedStringArray &out_warnings) const;
#endif

	void set_shader_passes(const TypedArray<ShaderPass> &p_passes);
	TypedArray<ShaderPass> get_shader_passes() const;

	void set_luts(const TypedArray<ShaderLUT> &p_luts);
	TypedArray<ShaderLUT> get_luts() const;

private:
	static void _bind_methods();

	Vector<Ref<ShaderPass>> passes;
	Vector<Ref<ShaderLUT>> luts;
};

VARIANT_ENUM_CAST(ShaderPassScale);
