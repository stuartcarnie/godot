/**************************************************************************/
/*  rendering_device_driver_metal4.mm                                     */
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

#ifdef METAL4_ENABLED

#import "rendering_device_driver_metal4.h"

#import "pixel_formats.h"
#import "rendering_context_driver_metal.h"

#import "core/config/project_settings.h"
#import "core/string/ustring.h"

#import <Metal/Metal.h>
#import <os/log.h>

#pragma mark - Logging

extern os_log_t LOG_DRIVER;

namespace API_AVAILABLE(macos(26.0), ios(26.0), tvos(26.0), visionos(26.0)) MTL4 {

/*****************/
/**** GENERIC ****/
/*****************/

#pragma mark - Fences

RDD::FenceID RenderingDeviceDriverMetal::fence_create() {
	Fence *fence = memnew(Fence((__bridge id<MTLSharedEvent>)device->newSharedEvent()));
	return FenceID(fence);
}

Error RenderingDeviceDriverMetal::fence_wait(FenceID p_fence) {
	Fence *fence = (Fence *)(p_fence.id);

	BOOL signaled = [fence->event waitUntilSignaledValue:fence->value timeoutMS:1000];
	if (!signaled) {
		ERR_PRINT("timeout waiting for fence");
	}

	return OK;
}

void RenderingDeviceDriverMetal::fence_free(FenceID p_fence) {
	Fence *fence = (Fence *)(p_fence.id);
	memdelete(fence);
}

#pragma mark - Semaphores

RDD::SemaphoreID RenderingDeviceDriverMetal::semaphore_create() {
	Semaphore *sem = memnew(Semaphore((__bridge id<MTLEvent>)device->newEvent()));
	return SemaphoreID(sem);
}

void RenderingDeviceDriverMetal::semaphore_free(SemaphoreID p_semaphore) {
	Semaphore *sem = (Semaphore *)(p_semaphore.id);
	memdelete(sem);
}

#pragma mark - Queues

RDD::CommandQueueID RenderingDeviceDriverMetal::command_queue_create(CommandQueueFamilyID p_cmd_queue_family, bool p_identify_as_main_queue) {
	if ((CommandQueueFamilyBits)p_cmd_queue_family.id == COMMAND_QUEUE_FAMILY_GRAPHICS_BIT || p_identify_as_main_queue) {
		return rid::make(device_queue);
	}
	return rid::make(transfer_queue);
}

Error RenderingDeviceDriverMetal::command_queue_execute_and_present(CommandQueueID p_cmd_queue, VectorView<SemaphoreID> p_wait_sem, VectorView<CommandBufferID> p_cmd_buffers, VectorView<SemaphoreID> p_cmd_sem, FenceID p_cmd_fence, VectorView<SwapChainID> p_swap_chains) {
	id<MTL4CommandQueue> queue = rid::get(p_cmd_queue);

	// If we have swap chains to present, this must be the device_queue.
	DEV_ASSERT((p_swap_chains.size() > 0 && queue == device_queue) || p_swap_chains.size() == 0);

	bool changed = false;
	id<MTLResidencySet> mrs = (__bridge id<MTLResidencySet>)main_residency_set.get();
	if (!_residency_add.is_empty()) {
		[mrs addAllocations:(id<MTLAllocation> *)_residency_add.ptr() count:_residency_add.size()];
		_residency_add.clear();
		changed = true;
	}
	if (!_residency_del.is_empty()) {
		[mrs removeAllocations:(id<MTLAllocation> *)_residency_del.ptr() count:_residency_del.size()];
		_residency_del.clear();
		changed = true;
	}
	if (changed) {
		[mrs commit];
	}

	uint32_t size = p_cmd_buffers.size();
	if (size == 0) {
		return OK;
	}

	for (uint32_t i = 0; i < p_wait_sem.size(); i++) {
		Semaphore *sem = (Semaphore *)p_wait_sem[i].id;
		[queue waitForEvent:sem->event value:sem->value];
	}

	if (size > 1) {
		uint32_t pre_commit_count = size - 1;
		id<MTL4CommandBuffer> __unsafe_unretained *cmds = ALLOCA_ARRAY(id<MTL4CommandBuffer> __unsafe_unretained, pre_commit_count);
		for (uint32_t i = 0; i < pre_commit_count; i++) {
			MDCommandBuffer *cmd_buffer = (MDCommandBuffer *)(p_cmd_buffers[i].id);
			cmd_buffer->commit();
			cmds[i] = cmd_buffer->get_command_buffer();
		}
		[queue commit:cmds count:pre_commit_count];
	}

	MDCommandBuffer *cmd_buffer = (MDCommandBuffer *)(p_cmd_buffers[size - 1].id);
	cmd_buffer->commit();

	id<MTLDrawable> __unsafe_unretained *drawables = ALLOCA_ARRAY(id<MTLDrawable> __unsafe_unretained, p_swap_chains.size());
	bzero(drawables, sizeof(id<MTLDrawable>) * p_swap_chains.size());
	for (uint32_t i = 0; i < p_swap_chains.size(); i++) {
		SwapChain *swap_chain = (SwapChain *)(p_swap_chains[i].id);
		RenderingContextDriverMetal::Surface *metal_surface = (RenderingContextDriverMetal::Surface *)(swap_chain->surface);
		id<MTLDrawable> drawable = (__bridge id<MTLDrawable>)metal_surface->next_drawable();
		if (drawable) {
			[queue waitForDrawable:drawable];
			drawables[i] = drawable;
		}
	}

	id<MTL4CommandBuffer> cb = cmd_buffer->get_command_buffer();
	[queue commit:&cb count:1];

	for (uint32_t i = 0; i < p_swap_chains.size(); i++) {
		if (drawables[i]) {
			SwapChain *swap_chain = (SwapChain *)(p_swap_chains[i].id);
			RenderingContextDriverMetal::Surface *metal_surface = (RenderingContextDriverMetal::Surface *)(swap_chain->surface);
			[queue signalDrawable:drawables[i]];
			if (metal_surface->vsync_mode != DisplayServer::VSYNC_DISABLED && metal_surface->present_minimum_duration > 0) {
				[drawables[i] presentAfterMinimumDuration:metal_surface->present_minimum_duration];
			} else {
				[drawables[i] present];
			}
		}
	}

	Fence *fence = (Fence *)(p_cmd_fence.id);
	if (fence != nullptr) {
		fence->value++;
		[queue signalEvent:fence->event value:fence->value];
	}

	for (uint32_t i = 0; i < p_cmd_sem.size(); i++) {
		Semaphore *sem = (Semaphore *)p_cmd_sem[i].id;
		sem->value++;
		[queue signalEvent:sem->event value:sem->value];
	}

	if (p_swap_chains.size() > 0) {
		// Used as a signal that we're presenting, so this is the end of a frame.
		id<MTLCaptureScope> scope = (__bridge id<MTLCaptureScope>)device_scope.get();
		[scope endScope];
		[scope beginScope];
	}

	return OK;
}

void RenderingDeviceDriverMetal::command_queue_free(CommandQueueID p_cmd_queue) {
}

#pragma mark - Command Buffers

// ----- POOL -----

RDD::CommandPoolID RenderingDeviceDriverMetal::command_pool_create(CommandQueueFamilyID p_cmd_queue_family, CommandBufferType p_cmd_buffer_type) {
	DEV_ASSERT(p_cmd_buffer_type == COMMAND_BUFFER_TYPE_PRIMARY);
	MD4CommandPool *obj = memnew(MD4CommandPool(this));
	command_pools.push_back(obj);
	return CommandPoolID(obj);
}

bool RenderingDeviceDriverMetal::command_pool_reset(CommandPoolID p_cmd_pool) {
	return true;
}

void RenderingDeviceDriverMetal::command_pool_free(CommandPoolID p_cmd_pool) {
	MD4CommandPool *obj = (MD4CommandPool *)(p_cmd_pool.id);
	int64_t pos = command_pools.find(obj);
	ERR_FAIL_COND_MSG(pos < 0, "MD4CommandPool does not belong to this device");
	command_pools.remove_at(pos);
	memdelete(obj);
}

// ----- BUFFER -----

RDD::CommandBufferID RenderingDeviceDriverMetal::command_buffer_create(CommandPoolID p_cmd_pool) {
	MD4CommandPool *pool = (MD4CommandPool *)(p_cmd_pool.id);
	MDCommandBuffer *obj = pool->new_command_buffer();
	return CommandBufferID(obj);
}

/******************/

RenderingDeviceDriverMetal::RenderingDeviceDriverMetal(RenderingContextDriverMetal *p_context_driver) :
		::RenderingDeviceDriverMetal(p_context_driver) {
}

RenderingDeviceDriverMetal::~RenderingDeviceDriverMetal() {
	for (MD4CommandPool *obj : command_pools) {
		memdelete(obj);
	}
}

#pragma mark - Initialization

Error RenderingDeviceDriverMetal::_create_device() {
	device = context_driver->get_metal_device();
	id<MTLDevice> mtl_device = (__bridge id<MTLDevice>)device;

	MTL4CommandQueueDescriptor *desc = [MTL4CommandQueueDescriptor new];
	desc.label = @"Main Queue";
	device_queue = [mtl_device newMTL4CommandQueueWithDescriptor:desc error:nil];
	ERR_FAIL_NULL_V_MSG(device_queue, ERR_CANT_CREATE, "Failed to create main queue");
	desc.label = @"Transfer Queue";
	transfer_queue = [mtl_device newMTL4CommandQueueWithDescriptor:desc error:nil];
	ERR_FAIL_NULL_V_MSG(transfer_queue, ERR_CANT_CREATE, "Failed to create transfer queue");

	id<MTLCaptureScope> scope = [MTLCaptureManager.sharedCaptureManager newCaptureScopeWithDevice:mtl_device];
	device_scope = NS::TransferPtr((__bridge_retained MTL::CaptureScope *)scope);
	scope.label = @"Godot Frame";
	[scope beginScope]; // Allow Xcode to capture the first frame, if desired.

	MTLResidencySetDescriptor *rs_desc = [MTLResidencySetDescriptor new];
	[rs_desc setInitialCapacity:10];
	rs_desc.label = @"Main Residency Set";
	NSError *error;
	id<MTLResidencySet> mrs = [mtl_device newResidencySetWithDescriptor:rs_desc error:&error];
	main_residency_set = NS::TransferPtr((__bridge_retained MTL::ResidencySet *)mrs);
	CRASH_COND_MSG(error != nil, vformat("Failed to create residency set: %s", String(error.localizedDescription.UTF8String)));

	[device_queue addResidencySet:mrs];
	[transfer_queue addResidencySet:mrs];

	return OK;
}

Error RenderingDeviceDriverMetal::initialize(uint32_t p_device_index, uint32_t p_frame_count) {
	// Call base class shared initialization.
	Error err = _initialize(p_device_index, p_frame_count);
	ERR_FAIL_COND_V(err, err);

	use_barriers = true;
	base_hazard_tracking = MTLResourceHazardTrackingModeUntracked;

	{
		MTL4CompilerDescriptor *desc = [MTL4CompilerDescriptor new];
		NSError *error = nil;
		compiler = [(__bridge id<MTLDevice>)device newCompilerWithDescriptor:desc error:&error];
	}

	return OK;
}

// Factory function for C++ compatibility
RenderingDeviceDriver *create_rendering_device_driver(RenderingContextDriverMetal *p_context) {
	return memnew(RenderingDeviceDriverMetal(p_context));
}

} //namespace API_AVAILABLE(macos(26.0),ios(26.0),tvos(26.0),visionos(26.0))MTL4

#endif // METAL4_ENABLED
