//
// Created by Stuart Carnie on 8/8/2024.
//

#ifndef GODOT_RETROFX_H
#define GODOT_RETROFX_H

#include "scene/main/node.h"

class RetroFX : public Node {
	GDCLASS(RetroFX, Node);

protected:
	void _notification(int p_what);
	static void _bind_methods();

	RetroFX();
};

#endif //GODOT_RETROFX_H
