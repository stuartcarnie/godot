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
#include <TargetConditionals.h>

#if defined(METAL_ENABLED) && !defined(VISIONOS_ENABLED) && !TARGET_OS_SIMULATOR
#define METAL_DISPLAY_LINK_ENABLED

#include <os/availability.h>

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

private:
	CA::MetalDisplayLink *link = nullptr;
	GodotMetalDisplayLinkDelegate *delegate = nullptr;
	Callback callback = nullptr;
	void *userdata = nullptr;
	int max_fps = -1;

public:
	// Attaches to p_layer and joins the current run loop. p_layer must be a CAMetalLayer with no
	// other link attached, and no link may be running.
	API_AVAILABLE(macos(14.0), ios(17.0), tvos(17.0))
	void start(CA::MetalLayer *p_layer, int p_frame_latency, Callback p_callback, void *p_userdata);
	void stop();

	// True while the link is attached to the layer. The layer refuses nextDrawable() in that state,
	// so every frame must use the drawable the link delivers.
	bool is_running() const { return link != nullptr; }
	void set_max_fps(int p_max_fps);

	// Called by the delegate.
	void _update(CA::MetalDrawable *p_drawable);

	~GodotMetalDisplayLink();
};

#endif // METAL_ENABLED && !VISIONOS_ENABLED && !TARGET_OS_SIMULATOR
