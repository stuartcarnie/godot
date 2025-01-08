//
// Created by Stuart Carnie on 8/8/2024.
//

#ifndef GODOT_SLANG_SHADER_H
#define GODOT_SLANG_SHADER_H

#include "core/io/resource.h"

/**
 * @brief A shader resource that uses the Slang shading language.
 */
class SlangShader : public Resource {
	GDCLASS(SlangShader, Resource);
};

#endif //GODOT_SLANG_SHADER_H
