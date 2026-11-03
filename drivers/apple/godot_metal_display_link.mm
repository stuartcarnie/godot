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
#include <TargetConditionals.h>

API_AVAILABLE(macos(14.0), ios(17.0), tvos(17.0))
@interface GodotMetalDisplayLinkDelegate : NSObject <CAMetalDisplayLinkDelegate>
@property(nonatomic, assign) GodotMetalDisplayLink *owner;
@end

@implementation GodotMetalDisplayLinkDelegate

- (void)metalDisplayLink:(CAMetalDisplayLink *)link needsUpdate:(CAMetalDisplayLinkUpdate *)update {
	if (self.owner == nullptr) {
		return;
	}
	CA::MetalDisplayLinkUpdate *cpp_update = (__bridge CA::MetalDisplayLinkUpdate *)update;
	self.owner->_update(cpp_update->drawable());
}

@end

bool GodotMetalDisplayLink::is_supported() {
#if TARGET_OS_SIMULATOR
	return false;
#else
	if (@available(macOS 14.0, iOS 17.0, tvOS 17.0, *)) {
		return true;
	}
	return false;
#endif
}

Error GodotMetalDisplayLink::start(CA::MetalLayer *p_layer, int p_frame_latency, Callback p_callback, StallCallback p_stall_callback, void *p_userdata) {
	ERR_FAIL_COND_V(link != nullptr, ERR_ALREADY_IN_USE);
	ERR_FAIL_NULL_V(p_layer, ERR_INVALID_PARAMETER);
	ERR_FAIL_NULL_V(p_callback, ERR_INVALID_PARAMETER);
	ERR_FAIL_COND_V(!is_supported(), ERR_UNAVAILABLE);

	if (@available(macOS 14.0, iOS 17.0, tvOS 17.0, *)) {
		delegate = [[GodotMetalDisplayLinkDelegate alloc] init];
		delegate.owner = this;

		link = CA::MetalDisplayLink::alloc()->init(p_layer);
		if (link == nullptr) {
			delegate = nil;
			ERR_FAIL_V_MSG(ERR_CANT_CREATE, "Could not create CAMetalDisplayLink.");
		}

		callback = p_callback;
		stall_callback = p_stall_callback;
		userdata = p_userdata;
		received_callback = false;

		link->setDelegate((__bridge NS::Object *)delegate);
		// The system accepts only 1 or 2.
		link->setPreferredFrameLatency(float(CLAMP(p_frame_latency, 1, 2)));
		link->addToRunLoop((__bridge NS::Object *)[NSRunLoop currentRunLoop], (__bridge NS::String *)NSRunLoopCommonModes);

		if (stall_callback != nullptr) {
			// stop() clears the delegate's owner, so a timer from an earlier start does nothing.
			__weak GodotMetalDisplayLinkDelegate *weak_delegate = delegate;
			dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC), dispatch_get_main_queue(), ^{
				GodotMetalDisplayLinkDelegate *strong_delegate = weak_delegate;
				if (strong_delegate != nil && strong_delegate.owner != nullptr) {
					strong_delegate.owner->_stall();
				}
			});
		}
		return OK;
	}

	return ERR_UNAVAILABLE;
}

void GodotMetalDisplayLink::stop() {
	if (link == nullptr) {
		return;
	}
	link->invalidate();
	link->release();
	link = nullptr;
	delegate.owner = nullptr;
	delegate = nil;
	callback = nullptr;
	stall_callback = nullptr;
	userdata = nullptr;
	received_callback = false;
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
	received_callback = true;
	if (callback) {
		callback(userdata, p_drawable);
	}
}

void GodotMetalDisplayLink::_stall() {
	if (received_callback || stall_callback == nullptr) {
		return;
	}
	// The callback may delete this link; nothing may touch members after it.
	stall_callback(userdata);
}

GodotMetalDisplayLink::~GodotMetalDisplayLink() {
	stop();
}

#endif // METAL_DISPLAY_LINK_ENABLED
