/**************************************************************************/
/*  godot_metal_display_link.mm                                           */
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

#import "godot_metal_display_link.h"

#if defined(METAL_DISPLAY_LINK_ENABLED)

#include "core/error/error_macros.h"
#include "core/math/math_funcs.h"

#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#include <QuartzCore/QuartzCore.hpp>

API_AVAILABLE(macos(14.0), ios(17.0), tvos(17.0))
@interface GodotMetalDisplayLinkDelegate : NSObject <CAMetalDisplayLinkDelegate> {
	GodotMetalDisplayLink *_owner;
}
- (instancetype)initWithOwner:(GodotMetalDisplayLink *)owner;
@end

@implementation GodotMetalDisplayLinkDelegate

- (instancetype)initWithOwner:(GodotMetalDisplayLink *)owner {
	self = [super init];
	if (self) {
		_owner = owner;
	}
	return self;
}

- (void)metalDisplayLink:(CAMetalDisplayLink *)link needsUpdate:(CAMetalDisplayLinkUpdate *)update {
	CA::MetalDisplayLinkUpdate *cpp_update = (__bridge CA::MetalDisplayLinkUpdate *)update;
	_owner->_update(cpp_update->drawable());
}

@end

void GodotMetalDisplayLink::start(CA::MetalLayer *p_layer, int p_frame_latency, Callback p_callback, void *p_userdata) {
	DEV_ASSERT(link == nullptr);
	DEV_ASSERT(p_layer != nullptr);
	DEV_ASSERT(p_callback != nullptr);

	delegate = [[GodotMetalDisplayLinkDelegate alloc] initWithOwner:this];
	link = CA::MetalDisplayLink::alloc()->init(p_layer);
	DEV_ASSERT(link != nullptr);

	callback = p_callback;
	userdata = p_userdata;

	link->setDelegate((__bridge NS::Object *)delegate);
	// The system accepts only 1 or 2.
	link->setPreferredFrameLatency(float(CLAMP(p_frame_latency, 1, 2)));
	link->addToRunLoop((__bridge NS::Object *)[NSRunLoop currentRunLoop], (__bridge NS::String *)NSRunLoopCommonModes);
}

void GodotMetalDisplayLink::stop() {
	if (link == nullptr) {
		return;
	}
	// invalidate() removes the link from the run loop, so the delegate gets no further callbacks.
	link->invalidate();
	link->release();
	link = nullptr;
	delegate = nil;
	callback = nullptr;
	userdata = nullptr;
	max_fps = -1;
}

void GodotMetalDisplayLink::set_max_fps(int p_max_fps) {
	if (link == nullptr || max_fps == p_max_fps) {
		return;
	}
	max_fps = p_max_fps;

	// {0, 0, 0} is CAFrameRateRangeDefault: the display's maximum refresh rate.
	// Any other range needs minimum > 0 or the setter throws.
	CA::FrameRateRange range = { 0.0f, 0.0f, 0.0f };
	if (p_max_fps > 0) {
		range.minimum = float(p_max_fps);
		range.maximum = float(p_max_fps);
		range.preferred = float(p_max_fps);
	}
	link->setPreferredFrameRateRange(range);
}

void GodotMetalDisplayLink::_update(CA::MetalDrawable *p_drawable) {
	callback(userdata, p_drawable);
}

GodotMetalDisplayLink::~GodotMetalDisplayLink() {
	stop();
}

#endif // METAL_DISPLAY_LINK_ENABLED
