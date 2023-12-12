/**************************************************************************/
/*  metal_context.h                                                       */
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

#ifndef METAL_CONTEXT_IMPL_H
#define METAL_CONTEXT_IMPL_H

#include "core/error/error_list.h"
#include "core/string/ustring.h"
#include "core/templates/rb_map.h"
#include "core/templates/rid_owner.h"
#include "metal_device_properties.h"
#include "rendering_device_driver_metal.h"
#include "servers/display_server.h"
#include "servers/rendering/renderer_rd/api_context_rd.h"

#import <Metal/Metal.h>
#import <QuartzCore/CALayer.h>

@class CAMetalLayer;
@protocol CAMetalDrawable;
class PixelFormats;
class MDResourceCache;

class MetalContext : public ApiContextRD {
private:
	id<MTLDevice> device;
	uint32_t version_major = 2;
	uint32_t version_minor = 0;
	MetalDeviceProperties *metal_device_properties = nullptr;
	PixelFormats *pixel_formats = nullptr;
	std::unique_ptr<MDResourceCache> resource_cache;

	RDD::MultiviewCapabilities multiview_capabilities;

	MTLPixelFormat format = MTLPixelFormatInvalid;

	struct Window {
		CAMetalLayer *layer = nil;
		std::shared_ptr<MDRenderPass> pass;
		std::shared_ptr<MDScreenFrameBuffer> frameBuffer;

		int width = 0;
		int height = 0;
		DisplayServer::VSyncMode vsync_mode = DisplayServer::VSYNC_ENABLED;
	};

	struct LocalDevice {
		bool waiting = false;
		id<MTLDevice> device = nil;
		id<MTLCommandQueue> queue = nil;
		RenderingDeviceDriverMetal *driver = nullptr;
		id<MTLCommandBuffer> command_buffer = nil;
	};

	RID_Owner<LocalDevice, true> local_device_owner;

	RenderingDeviceDriverMetal *driver = nullptr;

	HashMap<DisplayServer::WindowID, Window> windows;
	uint32_t swapchainImageCount = 0;

	// Commands.

	bool prepared = false;

	LocalVector<MDCommandBuffer *> command_buffer_queue;
	int command_buffer_count = 1;

	id<MTLCommandQueue> queue;
	id<MTLCaptureScope> scope;

	String device_vendor;
	String device_name;
	String pipeline_cache_id;

	Error _create_device();
	Error _update_swap_chain(Window *window);
	Error _check_capabilities();

protected:
	virtual Error _window_create(DisplayServer::WindowID p_window_id, DisplayServer::VSyncMode p_vsync_mode, CALayer *p_layer, int p_width, int p_height);

public:
	uint32_t get_version_major() const { return version_major; };
	uint32_t get_version_minor() const { return version_minor; };
	PixelFormats &get_pixel_formats() const { return *pixel_formats; }
	MDResourceCache &get_resource_cache() const { return *resource_cache; }

	char const *get_api_name() const final { return "Metal"; };
	RenderingDevice::Capabilities get_device_capabilities() const final;
	const RDD::MultiviewCapabilities &get_multiview_capabilities() const final { return multiview_capabilities; };

	id<MTLDevice> get_device() const;
	int get_swapchain_image_count() const final;
	id<MTLCommandQueue> get_graphics_queue() const;

	void window_resize(DisplayServer::WindowID p_window_id, int p_width, int p_height) final;
	int window_get_width(DisplayServer::WindowID p_window) final;
	int window_get_height(DisplayServer::WindowID p_window) final;
	bool window_is_valid_swapchain(DisplayServer::WindowID p_window) final;
	void window_destroy(DisplayServer::WindowID p_window_id) final;
	RDD::RenderPassID window_get_render_pass(DisplayServer::WindowID p_window) final;
	RDD::FramebufferID window_get_framebuffer(DisplayServer::WindowID p_window) final;

	RID local_device_create() final;
	void local_device_push_command_buffers(RID p_local_device, const RDD::CommandBufferID *p_buffers, int p_count) final;
	void local_device_sync(RID p_local_device) final;
	void local_device_free(RID p_local_device) final;

	MTLPixelFormat get_screen_format() const;
	MetalDeviceProperties const &get_device_properties() const { return *metal_device_properties; }

	_FORCE_INLINE_ uint32_t get_metal_buffer_index_for_vertex_attribute_binding(uint32_t binding) {
		return (metal_device_properties->limits.maxPerStageBufferCount - 1) - binding;
	}

	size_t get_texel_buffer_alignment_for_format(RDD::DataFormat p_format) const;

	void set_setup_buffer(RDD::CommandBufferID p_command_buffer) final;
	void append_command_buffer(RDD::CommandBufferID p_command_buffer) final;
	void resize_notify();
	void flush(bool p_flush_setup, bool p_flush_pending) final;
	Error prepare_buffers(RDD::CommandBufferID p_command_buffer) final;
	void postpare_buffers(RDD::CommandBufferID p_command_buffer) final;
	Error swap_buffers() final;
	Error initialize() final;

	void command_begin_label(RDD::CommandBufferID p_command_buffer, String p_label_name, const Color &p_color) final;
	void command_insert_label(RDD::CommandBufferID p_command_buffer, String p_label_name, const Color &p_color) final;
	void command_end_label(RDD::CommandBufferID p_command_buffer) final;

	String get_device_vendor_name() const final;
	String get_device_name() const final;
	RDD::DeviceType get_device_type() const final;
	String get_device_api_version() const final;
	String get_device_pipeline_cache_uuid() const final;

	void set_vsync_mode(DisplayServer::WindowID p_window, DisplayServer::VSyncMode p_mode) final;
	DisplayServer::VSyncMode get_vsync_mode(DisplayServer::WindowID p_window) const final;

	RenderingDeviceDriver *get_driver(RID p_local_device) final;

	MetalContext();
	~MetalContext() override;
};

#endif //METAL_CONTEXT_IMPL_H
