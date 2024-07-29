/**************************************************************************/
/*  rendering_context_driver_metal.h                                      */
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

#ifndef RENDERING_CONTEXT_DRIVER_METAL_H
#define RENDERING_CONTEXT_DRIVER_METAL_H

#ifdef METAL_ENABLED

#import "rendering_device_driver_metal.h"
#import "servers/rendering/rendering_context_driver.h"

#import <CoreGraphics/CGGeometry.h>
#import <Metal/Metal.h>
#import <QuartzCore/CALayer.h>

@class CAMetalLayer;
@protocol CAMetalDrawable;
class PixelFormats;
class MDResourceCache;

class API_AVAILABLE(macos(11.0), ios(14.0)) RenderingContextDriverMetal : public RenderingContextDriver {
protected:
	id<MTLDevice> metal_device = nil;
	Device device; // there is only one device on Apple Silicon (for now)

public:
	Error initialize() final override;
	const Device &device_get(uint32_t p_device_index) const final override;
	uint32_t device_get_count() const final override;
	bool device_supports_present(uint32_t p_device_index, SurfaceID p_surface) const final override { return true; }
	RenderingDeviceDriver *driver_create() final override;
	void driver_free(RenderingDeviceDriver *p_driver) final override;
	SurfaceID surface_create(const void *p_platform_data) final override;
	void surface_set_size(SurfaceID p_surface, uint32_t p_width, uint32_t p_height) final override;
	void surface_set_vsync_mode(SurfaceID p_surface, DisplayServer::VSyncMode p_vsync_mode) final override;
	DisplayServer::VSyncMode surface_get_vsync_mode(SurfaceID p_surface) const final override;
	uint32_t surface_get_width(SurfaceID p_surface) const final override;
	uint32_t surface_get_height(SurfaceID p_surface) const final override;
	void surface_set_needs_resize(SurfaceID p_surface, bool p_needs_resize) final override;
	bool surface_get_needs_resize(SurfaceID p_surface) const final override;
	void surface_destroy(SurfaceID p_surface) final override;
	bool is_debug_utils_enabled() const final override { return true; }

#pragma mark - Metal-specific methods

	// Platform-specific data for the Windows embedded in this driver.
	struct WindowPlatformData {
		CAMetalLayer *__unsafe_unretained layer;
	};

	class Surface {
	protected:
		id<MTLDevice> device;

	public:
		uint32_t width = 0;
		uint32_t height = 0;
		DisplayServer::VSyncMode vsync_mode = DisplayServer::VSYNC_ENABLED;
		bool needs_resize = false;

		Surface(id<MTLDevice> p_device) :
				device(p_device) {}
		virtual ~Surface() = default;

		MTLPixelFormat get_pixel_format() const { return MTLPixelFormatBGRA8Unorm; }
		virtual Error resize(uint32_t p_desired_framebuffer_count) = 0;
		virtual RDD::FramebufferID acquire_next_frame_buffer() = 0;
		virtual void present(MDCommandBuffer *p_cmd_buffer) = 0;
	};

	class SurfaceLayer : public Surface {
		CAMetalLayer *__unsafe_unretained layer = nil;
		LocalVector<MDFrameBuffer> frame_buffers;
		LocalVector<id<MTLDrawable>> drawables;
		uint32_t rear = -1;
		uint32_t front = 0;
		uint32_t count = 0;

	public:
		SurfaceLayer(CAMetalLayer *p_layer, id<MTLDevice> p_device) :
				Surface(p_device), layer(p_layer) {
			layer.allowsNextDrawableTimeout = YES;
			layer.framebufferOnly = YES;
			layer.opaque = OS::get_singleton()->is_layered_allowed() ? NO : YES;
			layer.pixelFormat = get_pixel_format();
			layer.device = p_device;
		}

		~SurfaceLayer() override {
			layer = nil;
		}

		Error resize(uint32_t p_desired_framebuffer_count) override final {
			if (width == 0 || height == 0) {
				// Very likely the window is minimized, don't create a swap chain.
				return ERR_SKIP;
			}

			CGSize drawableSize = CGSizeMake(width, height);
			CGSize current = layer.drawableSize;
			if (!CGSizeEqualToSize(current, drawableSize)) {
				layer.drawableSize = drawableSize;
			}

			// Metal supports a maximum of 3 drawables
			p_desired_framebuffer_count = MIN(3U, p_desired_framebuffer_count);
			layer.maximumDrawableCount = p_desired_framebuffer_count;

#if TARGET_OS_OSX
			// display sync is only supported on macOS
			switch (vsync_mode) {
				case DisplayServer::VSYNC_MAILBOX:
				case DisplayServer::VSYNC_ADAPTIVE:
				case DisplayServer::VSYNC_ENABLED:
					layer.displaySyncEnabled = YES;
					break;
				case DisplayServer::VSYNC_DISABLED:
					layer.displaySyncEnabled = NO;
					break;
			}
#endif
			drawables.resize(p_desired_framebuffer_count);
			frame_buffers.resize(p_desired_framebuffer_count);
			for (uint32_t i = 0; i < p_desired_framebuffer_count; i++) {
				// reserve space for the drawable texture
				frame_buffers[i].textures.resize(1);
			}

			return OK;
		}

		RDD::FramebufferID acquire_next_frame_buffer() override final {
			if (count == frame_buffers.size()) {
				return RDD::FramebufferID();
			}

			rear = (rear + 1) % frame_buffers.size();
			count++;

			MDFrameBuffer &frame_buffer = frame_buffers[rear];
			frame_buffer.size = Size2i(width, height);

			id<CAMetalDrawable> drawable = layer.nextDrawable;
			ERR_FAIL_NULL_V_MSG(drawable, RDD::FramebufferID(), "no drawable available");
			drawables[rear] = drawable;
			frame_buffer.textures.write[0] = drawable.texture;

			return RDD::FramebufferID(&frame_buffer);
		}

		void present(MDCommandBuffer *p_cmd_buffer) override final {
			if (count == 0) {
				return;
			}

			// release texture and drawable
			frame_buffers[front].textures.write[0] = nil;
			id<MTLDrawable> drawable = drawables[front];
			drawables[front] = nil;

			count--;
			front = (front + 1) % frame_buffers.size();

			[p_cmd_buffer->get_command_buffer() presentDrawable:drawable];
		}
	};

	class SurfaceOffscreen : public Surface {
		int frame_buffer_size = 3;
		MDFrameBuffer *frame_buffers;
		LocalVector<id<MTLTexture>> textures;
		LocalVector<id<MTLDrawable>> drawables;

		int32_t rear = -1;
		std::atomic_int count;
		uint64_t target_time = 0;
		CAMetalLayer *layer;

	public:
		SurfaceOffscreen(CAMetalLayer *p_layer, id<MTLDevice> p_device) :
				Surface(p_device), layer(p_layer) {
			layer.allowsNextDrawableTimeout = YES;
			layer.framebufferOnly = YES;
			layer.opaque = OS::get_singleton()->is_layered_allowed() ? NO : YES;
			layer.pixelFormat = get_pixel_format();
			layer.device = p_device;
#if TARGET_OS_OSX
			layer.displaySyncEnabled = NO;
#endif
			target_time = OS::get_singleton()->get_ticks_usec();

			textures.resize(frame_buffer_size);
			drawables.resize(frame_buffer_size);

			frame_buffers = memnew_arr(MDFrameBuffer, frame_buffer_size);
			for (int i = 0; i < frame_buffer_size; i++) {
				frame_buffers[i].textures.resize(1);
			}
		}

		~SurfaceOffscreen() override {
			memdelete_arr(frame_buffers);
		}

		Error resize(uint32_t p_desired_framebuffer_count) override final {
			if (width == 0 || height == 0) {
				// Very likely the window is minimized, don't create a swap chain.
				return ERR_SKIP;
			}

			CGSize drawableSize = CGSizeMake(width, height);
			CGSize current = layer.drawableSize;
			if (!CGSizeEqualToSize(current, drawableSize)) {
				layer.drawableSize = drawableSize;
			}

			return OK;
		}

		RDD::FramebufferID acquire_next_frame_buffer() override final {
			if (count.load(std::memory_order_relaxed) == 3) {
				// Wait for a frame to be presented.
				return RDD::FramebufferID();
			}

			rear = (rear + 1) % 3;
			count.fetch_add(1, std::memory_order_relaxed);

			MDFrameBuffer &frame_buffer = frame_buffers[rear];

			if (textures[rear] == nil || textures[rear].width != width || textures[rear].height != height) {
				MTLTextureDescriptor *texture_descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:get_pixel_format() width:width height:height mipmapped:NO];
				texture_descriptor.usage = MTLTextureUsageRenderTarget;
				texture_descriptor.hazardTrackingMode = MTLHazardTrackingModeUntracked;
				texture_descriptor.storageMode = MTLStorageModePrivate;
				textures[rear] = [device newTextureWithDescriptor:texture_descriptor];
			}

			frame_buffer.size = Size2i(width, height);
			uint64_t now = OS::get_singleton()->get_ticks_usec();
			if (now >= target_time) {
				target_time = now + 1'000'000; // 1 second into the future
				id<CAMetalDrawable> drawable = layer.nextDrawable;
				ERR_FAIL_NULL_V_MSG(drawable, RDD::FramebufferID(), "no drawable available");
				drawables[rear] = drawable;
				frame_buffer.textures.write[0] = drawable.texture;
			} else {
				frame_buffer.textures.write[0] = textures[rear];
			}

			return RDD::FramebufferID(&frame_buffers[rear]);
		}

		void present(MDCommandBuffer *p_cmd_buffer) override final {
			MDFrameBuffer *frame_buffer = &frame_buffers[rear];

			if (drawables[rear] != nil) {
				[p_cmd_buffer->get_command_buffer() presentDrawable:drawables[rear]];
				drawables[rear] = nil;
			}

			[p_cmd_buffer->get_command_buffer() addScheduledHandler:^(id<MTLCommandBuffer> p_command_buffer) {
				frame_buffer->textures.write[0] = nil;
				count.fetch_add(-1, std::memory_order_relaxed);
			}];
		}
	};

	id<MTLDevice> get_metal_device() const { return metal_device; }

#pragma mark - Initialization

	RenderingContextDriverMetal();
	~RenderingContextDriverMetal() override;
};

#endif // METAL_ENABLED

#endif // RENDERING_CONTEXT_DRIVER_METAL_H
