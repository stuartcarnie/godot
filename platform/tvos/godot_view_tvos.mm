/**************************************************************************/
/*  godot_view_tvos.mm                                                    */
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

#import "godot_view_tvos.h"

#import "display_layer_tvos.h"
#import "display_server_tvos.h"

#include "core/error/error_macros.h"

#define Key GC_Key_
#import <GameController/GameController.h>
#undef Key

@interface GDTViewTVOS ()

GODOT_CLANG_WARNING_PUSH_AND_IGNORE("-Wobjc-property-synthesis")
@property(strong, nonatomic) CALayer<GDTDisplayLayer> *renderingLayer;
GODOT_CLANG_WARNING_POP

@end

@implementation GDTViewTVOS

- (void)godot_commonInit {
	[super godot_commonInit];
	self.contentScaleFactor = 1.0;
	self.userInteractionEnabled = YES;

	UISwipeGestureRecognizerDirection directions[] = {
		UISwipeGestureRecognizerDirectionLeft,
		UISwipeGestureRecognizerDirectionRight,
		UISwipeGestureRecognizerDirectionUp,
		UISwipeGestureRecognizerDirectionDown,
	};
	for (int i = 0; i < 4; i++) {
		UISwipeGestureRecognizer *swipe = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(godot_remoteSwipe:)];
		swipe.direction = directions[i];
		[self addGestureRecognizer:swipe];
	}
}

- (BOOL)canBecomeFocused {
	return YES;
}

- (void)didMoveToWindow {
	[super didMoveToWindow];
	if (self.window) {
		[self setNeedsFocusUpdate];
		[self updateFocusIfNeeded];
	}
}

- (void)godot_sendRemoteKey:(Key)key pressed:(BOOL)pressed {
	DisplayServerTVOS *display_server = DisplayServerTVOS::get_singleton();
	if (!display_server) {
		return;
	}
	display_server->key(key, 0, key, key, 0, pressed, KeyLocation::UNSPECIFIED);
}

- (void)godot_tapRemoteKey:(Key)key {
	[self godot_sendRemoteKey:key pressed:YES];
	[self godot_sendRemoteKey:key pressed:NO];
}

- (void)godot_remoteSwipe:(UISwipeGestureRecognizer *)sender {
	switch (sender.direction) {
		case UISwipeGestureRecognizerDirectionLeft:
			[self godot_tapRemoteKey:Key::LEFT];
			break;
		case UISwipeGestureRecognizerDirectionRight:
			[self godot_tapRemoteKey:Key::RIGHT];
			break;
		case UISwipeGestureRecognizerDirectionUp:
			[self godot_tapRemoteKey:Key::UP];
			break;
		case UISwipeGestureRecognizerDirectionDown:
			[self godot_tapRemoteKey:Key::DOWN];
			break;
		default:
			break;
	}
}

- (void)pressesBegan:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event {
	BOOL handled = NO;
	for (UIPress *press in presses) {
		switch (press.type) {
			case UIPressTypeSelect:
				[self godot_sendRemoteKey:Key::ENTER pressed:YES];
				handled = YES;
				break;
			case UIPressTypeMenu:
				[self godot_sendRemoteKey:Key::ESCAPE pressed:YES];
				handled = YES;
				break;
			case UIPressTypePlayPause:
				[self godot_sendRemoteKey:Key::SPACE pressed:YES];
				handled = YES;
				break;
			default:
				break;
		}
	}
	if (!handled) {
		[super pressesBegan:presses withEvent:event];
	}
}

- (void)pressesEnded:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event {
	BOOL handled = NO;
	for (UIPress *press in presses) {
		switch (press.type) {
			case UIPressTypeSelect:
				[self godot_sendRemoteKey:Key::ENTER pressed:NO];
				handled = YES;
				break;
			case UIPressTypeMenu:
				[self godot_sendRemoteKey:Key::ESCAPE pressed:NO];
				handled = YES;
				break;
			case UIPressTypePlayPause:
				[self godot_sendRemoteKey:Key::SPACE pressed:NO];
				handled = YES;
				break;
			default:
				break;
		}
	}
	if (!handled) {
		[super pressesEnded:presses withEvent:event];
	}
}

- (void)pressesCancelled:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event {
	BOOL handled = NO;
	for (UIPress *press in presses) {
		switch (press.type) {
			case UIPressTypeSelect:
				[self godot_sendRemoteKey:Key::ENTER pressed:NO];
				handled = YES;
				break;
			case UIPressTypeMenu:
				[self godot_sendRemoteKey:Key::ESCAPE pressed:NO];
				handled = YES;
				break;
			case UIPressTypePlayPause:
				[self godot_sendRemoteKey:Key::SPACE pressed:NO];
				handled = YES;
				break;
			default:
				break;
		}
	}
	if (!handled) {
		[super pressesCancelled:presses withEvent:event];
	}
}

- (CALayer<GDTDisplayLayer> *)initializeRenderingForDriver:(NSString *)driverName {
	if (self.renderingLayer) {
		return self.renderingLayer;
	}

	CALayer<GDTDisplayLayer> *layer;

	if ([driverName isEqualToString:@"metal"]) {
		layer = [GDTMetalLayer layer];
#if defined(GLES3_ENABLED)
	} else if ([driverName isEqualToString:@"opengl3"]) {
		GODOT_CLANG_WARNING_PUSH_AND_IGNORE("-Wdeprecated-declarations")
		layer = [GDTOpenGLLayer layer];
		GODOT_CLANG_WARNING_POP
#endif
	} else {
		return nil;
	}

	layer.frame = self.bounds;
	layer.contentsScale = self.contentScaleFactor;

	[self.layer addSublayer:layer];
	self.renderingLayer = layer;

	[layer initializeDisplayLayer];

	return self.renderingLayer;
}

@end

GDTView *GDTViewCreate() {
	GDTViewTVOS *view = [GDTViewTVOS new];
	view.preferredFrameRate = 60;
	return view;
}
