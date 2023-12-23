/**************************************************************************/
/*  metal_context.mm                                                      */
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

#import "metal_context.h"

#include "core/config/project_settings.h"
#import "metal_objects.h"
#import "pixel_formats.h"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include <memory>

Error MetalContext::_create_device() {
	device = MTLCreateSystemDefaultDevice();
	ERR_FAIL_NULL_V(device, ERR_CANT_CREATE);
	queue = device.newCommandQueue;
	ERR_FAIL_NULL_V(queue, ERR_CANT_CREATE);
	scope = [MTLCaptureManager.sharedCaptureManager newCaptureScopeWithCommandQueue:queue];
	scope.label = @"Metal Context";

	resource_cache = std::make_unique<MDResourceCache>(device, this);

	return OK;
}

Error MetalContext::_check_capabilities() {
	MTLCompileOptions *options = [MTLCompileOptions new];
	version_major = (options.languageVersion >> 0x10) & 0xff;
	version_minor = (options.languageVersion >> 0x00) & 0xff;

	return OK;
}

Error MetalContext::_window_create(DisplayServer::WindowID p_window_id, DisplayServer::VSyncMode p_vsync_mode, CALayer *p_layer, int p_width, int p_height) {
	ERR_FAIL_COND_V(windows.has(p_window_id), ERR_INVALID_PARAMETER);

	if (![p_layer isKindOfClass:CAMetalLayer.class]) {
		ERR_FAIL_V_MSG(ERR_CANT_CREATE, "Can't create a Metal context: expected CAMetalLayer");
	}

	CAMetalLayer *metal_layer = (CAMetalLayer *)p_layer;
	metal_layer.device = device;

	metal_layer.allowsNextDrawableTimeout = YES;
	metal_layer.framebufferOnly = YES;

	Window window;
	window.layer = metal_layer;
	window.width = p_width;
	window.height = p_height;
	window.vsync_mode = p_vsync_mode;
	Error err = _update_swap_chain(&window);
	ERR_FAIL_COND_V(err != OK, ERR_CANT_CREATE);

	windows[p_window_id] = window;

	return OK;
}

void MetalContext::window_resize(DisplayServer::WindowID p_window, int p_width, int p_height) {
	ERR_FAIL_COND(!windows.has(p_window));
	windows[p_window].width = p_width;
	windows[p_window].height = p_height;
	_update_swap_chain(&windows[p_window]);
}

int MetalContext::window_get_width(DisplayServer::WindowID p_window) {
	ERR_FAIL_COND_V(!windows.has(p_window), -1);
	return windows[p_window].width;
}

int MetalContext::window_get_height(DisplayServer::WindowID p_window) {
	ERR_FAIL_COND_V(!windows.has(p_window), -1);
	return windows[p_window].height;
}

bool MetalContext::window_is_valid_swapchain(DisplayServer::WindowID p_window) {
	ERR_FAIL_COND_V(!windows.has(p_window), false);
	Window *w = &windows[p_window];
	return w->layer != nil;
}

RDD::RenderPassID MetalContext::window_get_render_pass(DisplayServer::WindowID p_window) {
	ERR_FAIL_COND_V(!windows.has(p_window), RDD::RenderPassID());
	Window *w = &windows[p_window];
	return RDD::RenderPassID(w->pass.get());
}

RDD::FramebufferID MetalContext::window_get_framebuffer(DisplayServer::WindowID p_window) {
	ERR_FAIL_COND_V(!windows.has(p_window), RDD::FramebufferID());
	Window *w = &windows[p_window];

	if (w->frameBuffer == nullptr) {
		id<CAMetalDrawable> drawable = w->layer.nextDrawable;
		ERR_FAIL_NULL_V_MSG(drawable, RDD::FramebufferID(), "no drawable available");
		CGSize size = w->layer.drawableSize;
		w->frameBuffer = std::make_shared<MDScreenFrameBuffer>(drawable, Size2i(size.width, size.height));
		return RDD::FramebufferID(w->frameBuffer.get());
	}
	return RDD::FramebufferID();
}

void MetalContext::window_destroy(DisplayServer::WindowID p_window_id) {
	ERR_FAIL_COND(!windows.has(p_window_id));

	windows.erase(p_window_id);
}

Error MetalContext::_update_swap_chain(Window *window) {
	CGSize drawableSize = CGSizeMake(window->width, window->height);
	CGSize current = window->layer.drawableSize;
	if (!CGSizeEqualToSize(current, drawableSize)) {
		window->layer.drawableSize = drawableSize;
	}

#if TARGET_OS_OSX
	// display sync is only supported on macOS
	switch (window->vsync_mode) {
		case DisplayServer::VSYNC_MAILBOX:
		case DisplayServer::VSYNC_ADAPTIVE:
		case DisplayServer::VSYNC_ENABLED:
			window->layer.displaySyncEnabled = YES;
			break;
		case DisplayServer::VSYNC_DISABLED:
			window->layer.displaySyncEnabled = NO;
			break;
	}
#endif

	format = window->layer.pixelFormat;

	TightLocalVector<MDAttachment> attachments;
	{
		MDAttachment color;
		color.type = MDAttachmentType::Color;
		color.format = window->layer.pixelFormat;
		color.loadAction = MTLLoadActionClear;
		color.storeAction = MTLStoreActionStore;
		attachments.push_back(color);
	}
	TightLocalVector<MDSubpass> subpasses;
	{
		MDSubpass subpass = { .subpass_index = 0 };
		{
			RDD::AttachmentReference color_ref;
			{
				color_ref.attachment = 0;
				color_ref.aspect.set_flag(RDD::TEXTURE_ASPECT_COLOR_BIT);
			}
			subpass.color_references.push_back(color_ref);
		}
		subpasses.push_back(subpass);
	}
	window->pass = std::make_shared<MDRenderPass>(std::move(attachments), std::move(subpasses));

	return OK;
}

Error MetalContext::initialize() {
	Error err = _create_device();
	ERR_FAIL_COND_V(err, ERR_CANT_CREATE);
	driver = memnew(RenderingDeviceDriverMetal(this, device));

	err = _check_capabilities();
	ERR_FAIL_COND_V(err, ERR_CANT_CREATE);

	metal_device_properties = memnew(MetalDeviceProperties(device));
	pixel_formats = memnew(PixelFormats(device));

	String rendering_method;
	if (OS::get_singleton()->get_current_rendering_method() == "mobile") {
		rendering_method = "Forward Mobile";
	} else {
		rendering_method = "Forward+";
	}

	String gpu_family = "Apple1";
	MTLGPUFamily family = metal_device_properties->features.highestFamily;
	if (family >= MTLGPUFamilyApple1 && family <= MTLGPUFamilyApple9) {
		int version = family - MTLGPUFamilyApple1 + 1;
		gpu_family = vformat("Apple%d", version);
	}

	print_line(vformat("Metal API %s (%s) - %s - %s",
			get_device_api_version(), gpu_family,
			rendering_method,
			metal_device_properties->device_name));

	// Check required features and abort if any of them is missing.
	if (!metal_device_properties->features.imageCubeArray) {
		// NOTE: Apple A11 (Apple4) GPUs support image cube arrays, which are devices from 2017 and newer.
		String error_string = vformat("Your Apple GPU does not support the following features which are required to use Metal-based renderers in Godot:\n\n");
		if (!metal_device_properties->features.imageCubeArray) {
			error_string += "- No support for image cube arrays.\n";
		}

#if defined(IOS_ENABLED)
		// iOS platform ports currently don't exit themselves when this method returns `ERR_CANT_CREATE`.
		OS::get_singleton()->alert(error_string + "\nClick OK to exit (black screen will be visible).");
#else
		OS::get_singleton()->alert(error_string + "\nClick OK to exit.");
#endif

		return ERR_CANT_CREATE;
	}

	return OK;
}

size_t MetalContext::get_texel_buffer_alignment_for_format(RDD::DataFormat p_format) const {
	return [device minimumLinearTextureAlignmentForPixelFormat:pixel_formats->getMTLPixelFormat(p_format)];
}

size_t MetalContext::get_texel_buffer_alignment_for_format(MTLPixelFormat p_format) const {
	return [device minimumLinearTextureAlignmentForPixelFormat:p_format];
}

void MetalContext::set_setup_buffer(RDD::CommandBufferID p_command_buffer) {
	MDCommandBuffer *obj = (MDCommandBuffer *)(p_command_buffer.id);
	command_buffer_queue[0] = obj;
}

void MetalContext::append_command_buffer(RDD::CommandBufferID p_command_buffer) {
	MDCommandBuffer *obj = (MDCommandBuffer *)(p_command_buffer.id);

	if (command_buffer_queue.size() <= command_buffer_count) {
		command_buffer_queue.resize(command_buffer_count + 1);
	}

	command_buffer_queue[command_buffer_count] = obj;
	command_buffer_count++;
}

void MetalContext::flush(bool p_flush_setup, bool p_flush_pending, bool p_sync) {
	id<MTLCommandBuffer> last = nil;
	if (p_flush_setup && command_buffer_queue[0]) {
		last = command_buffer_queue[0]->get_command_buffer();
		command_buffer_queue[0]->commit();
		command_buffer_queue[0] = nullptr;
	}

	if (p_flush_pending && command_buffer_count > 1) {
		for (int i = 1; i < command_buffer_count; i++) {
			last = command_buffer_queue[i]->get_command_buffer();
			command_buffer_queue[i]->commit();
			command_buffer_queue[i] = nullptr;
		}
		command_buffer_count = 1;
	}
	if (p_sync) {
		[last waitUntilCompleted];
	}
}

Error MetalContext::prepare_buffers(RDD::CommandBufferID p_command_buffer) {
	[scope beginScope];
	return OK;
}

void MetalContext::postpare_buffers(RDD::CommandBufferID p_command_buffer) {
}

Error MetalContext::swap_buffers() {
	if (command_buffer_queue[0] != nullptr) {
		command_buffer_queue[0]->commit();
		command_buffer_queue[0] = nullptr;
	}

	if (command_buffer_count > 1) {
		int last = command_buffer_count - 1;
		for (int i = 1; i < last; i++) {
			command_buffer_queue[i]->commit();
			command_buffer_queue[i] = nil;
		}

		// last command buffer is used for presentation
		MDCommandBuffer *cb = command_buffer_queue[last];
		command_buffer_queue[last] = nil;
		for (KeyValue<int, Window> &E : windows) {
			Window *w = &E.value;
			if (w->frameBuffer) {
				[cb->get_command_buffer() presentDrawable:w->frameBuffer->drawable];
				w->frameBuffer = nil;
			}
		}
		cb->commit();

		command_buffer_count = 1;
	}

	[scope endScope];

	return OK;
}

void MetalContext::resize_notify() {
}

RenderingDevice::Capabilities MetalContext::get_device_capabilities() const {
	RenderingDevice::Capabilities c;
	c.device_family = RenderingDevice::DEVICE_METAL;
	c.version_major = version_major;
	c.version_minor = version_minor;
	return c;
}

id<MTLDevice> MetalContext::get_device() const {
	return device;
}

int MetalContext::get_swapchain_image_count() const {
	return 3;
}

id<MTLCommandQueue> MetalContext::get_graphics_queue() const {
	return queue;
}

MTLPixelFormat MetalContext::get_screen_format() const {
	return format;
}

RID MetalContext::local_device_create() {
	LocalDevice ld;
	ld.device = device;

	ld.driver = memnew(RenderingDeviceDriverMetal(this, ld.device));

	return local_device_owner.make_rid(ld);
}

void MetalContext::local_device_push_command_buffers(RID p_local_device, const RDD::CommandBufferID *p_buffers, int p_count) {
	LocalDevice *ld = local_device_owner.get_or_null(p_local_device);
	ERR_FAIL_COND(ld->waiting);
	ld->waiting = true;

	// capture the last
	{
		MDCommandBuffer *cb = (MDCommandBuffer *)(p_buffers[p_count - 1].id);
		ld->command_buffer = cb->get_command_buffer();
	}
	for (int i = 0; i < p_count; i++) {
		MDCommandBuffer *cb = (MDCommandBuffer *)(p_buffers[i].id);
		cb->commit();
	}
}

void MetalContext::local_device_sync(RID p_local_device) {
	LocalDevice *ld = local_device_owner.get_or_null(p_local_device);
	ERR_FAIL_COND(!ld->waiting);
	[ld->command_buffer waitUntilCompleted];
	ld->command_buffer = nil;
	ld->waiting = false;
}

void MetalContext::local_device_free(RID p_local_device) {
	LocalDevice *ld = local_device_owner.get_or_null(p_local_device);
	memdelete(ld->driver);
	local_device_owner.free(p_local_device);
}

String MetalContext::get_device_vendor_name() const {
	return metal_device_properties->device_vendor;
}

String MetalContext::get_device_name() const {
	return metal_device_properties->device_name;
}

RenderingDevice::DeviceType MetalContext::get_device_type() const {
	return RenderingDevice::DEVICE_TYPE_INTEGRATED_GPU;
}

String MetalContext::get_device_api_version() const {
	return vformat("%d.%d", version_major, version_minor);
}

String MetalContext::get_device_pipeline_cache_uuid() const {
	return pipeline_cache_id;
}

DisplayServer::VSyncMode MetalContext::get_vsync_mode(DisplayServer::WindowID p_window) const {
	ERR_FAIL_COND_V_MSG(!windows.has(p_window), DisplayServer::VSYNC_ENABLED, "Could not get V-Sync mode for window with WindowID " + itos(p_window) + " because it does not exist.");
	return windows[p_window].vsync_mode;
}

void MetalContext::set_vsync_mode(DisplayServer::WindowID p_window, DisplayServer::VSyncMode p_mode) {
	ERR_FAIL_COND_MSG(!windows.has(p_window), "Could not set V-Sync mode for window with WindowID " + itos(p_window) + " because it does not exist.");
	windows[p_window].vsync_mode = p_mode;
	_update_swap_chain(&windows[p_window]);
}

RenderingDeviceDriver *MetalContext::get_driver(RID p_local_device) {
	if (p_local_device.is_valid()) {
		LocalDevice *ld = local_device_owner.get_or_null(p_local_device);
		ERR_FAIL_NULL_V(ld, nullptr);
		return ld->driver;
	} else {
		return driver;
	}
}

MetalContext::MetalContext() {
	command_buffer_queue.resize(1); // First one is always the setup command.
}

MetalContext::~MetalContext() {
	if (metal_device_properties) {
		memdelete(metal_device_properties);
	}
	if (pixel_formats) {
		memdelete(pixel_formats);
	}
}
