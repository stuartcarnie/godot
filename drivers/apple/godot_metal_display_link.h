/**************************************************************************/
/*  godot_metal_display_link.h                                            */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

// visionOS paces frames through Compositor Services and never uses the link.
#if defined(METAL_ENABLED) && !defined(VISIONOS_ENABLED)
#define METAL_DISPLAY_LINK_ENABLED

#include "core/error/error_list.h"

namespace CA {
class MetalDisplayLink;
class MetalDrawable;
class MetalLayer;
} // namespace CA

@class GodotMetalDisplayLinkDelegate;

// Paces the main loop with a CAMetalDisplayLink attached to a CAMetalLayer.
// The callback runs on the main thread once per display refresh and receives
// the drawable the frame must present to.
class GodotMetalDisplayLink {
public:
	typedef void (*Callback)(void *p_userdata, CA::MetalDrawable *p_drawable);
	// Runs on the main queue when the link has delivered no frame one second after start().
	typedef void (*StallCallback)(void *p_userdata);

private:
	CA::MetalDisplayLink *link = nullptr;
	GodotMetalDisplayLinkDelegate *delegate = nullptr;
	Callback callback = nullptr;
	StallCallback stall_callback = nullptr;
	void *userdata = nullptr;
	int max_fps = -1;
	bool received_callback = false;

public:
	// False before macOS 14, iOS 17 and tvOS 17, and on the simulator.
	static bool is_supported();

	// p_stall_callback may be null. It may stop or delete this link.
	Error start(CA::MetalLayer *p_layer, int p_frame_latency, Callback p_callback, StallCallback p_stall_callback, void *p_userdata);
	void stop();

	// True while the link is attached to the layer. The layer refuses nextDrawable() in that state,
	// so every frame must use the drawable the link delivers.
	bool is_running() const { return link != nullptr; }
	void set_max_fps(int p_max_fps);

	// Called by the delegate and the stall timer.
	void _update(CA::MetalDrawable *p_drawable);
	void _stall();

	~GodotMetalDisplayLink();
};

#endif // METAL_ENABLED && !VISIONOS_ENABLED
