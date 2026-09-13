/**************************************************************************/
/*  rendering_device_driver_metal4.cpp                                    */
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

#include "rendering_device_driver_metal4.h"

#include "pixel_formats.h"
#include "rendering_context_driver_metal.h"

#include "core/string/ustring.h"

namespace API_AVAILABLE(macos(26.0), ios(26.0), tvos(26.0), visionos(26.0)) MTL4 {

/*****************/
/**** GENERIC ****/
/*****************/

#pragma mark - Fences

RDD::FenceID RenderingDeviceDriverMetal::fence_create() {
	Fence *fence = memnew(Fence(NS::TransferPtr(device->newSharedEvent())));
	return FenceID(fence);
}

Error RenderingDeviceDriverMetal::fence_wait(FenceID p_fence) {
	Fence *fence = (Fence *)(p_fence.id);

	bool signaled = fence->event->waitUntilSignaledValue(fence->value, 1000);
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
	Semaphore *sem = memnew(Semaphore(NS::TransferPtr(device->newEvent())));
	return SemaphoreID(sem);
}

void RenderingDeviceDriverMetal::semaphore_free(SemaphoreID p_semaphore) {
	Semaphore *sem = (Semaphore *)(p_semaphore.id);
	memdelete(sem);
}

#pragma mark - Queues

RDD::CommandQueueID RenderingDeviceDriverMetal::command_queue_create(CommandQueueFamilyID p_cmd_queue_family, bool p_identify_as_main_queue) {
	if ((CommandQueueFamilyBits)p_cmd_queue_family.id == COMMAND_QUEUE_FAMILY_GRAPHICS_BIT || p_identify_as_main_queue) {
		return CommandQueueID(reinterpret_cast<uint64_t>(device_queue.get()));
	}
	return CommandQueueID(reinterpret_cast<uint64_t>(transfer_queue.get()));
}

Error RenderingDeviceDriverMetal::command_queue_execute_and_present(CommandQueueID p_cmd_queue, VectorView<SemaphoreID> p_wait_sem, VectorView<CommandBufferID> p_cmd_buffers, VectorView<SemaphoreID> p_cmd_sem, FenceID p_cmd_fence, VectorView<SwapChainID> p_swap_chains) {
	MTL4::CommandQueue *queue = reinterpret_cast<MTL4::CommandQueue *>(p_cmd_queue.id);

	// If we have swap chains to present, this must be the device_queue.
	DEV_ASSERT((p_swap_chains.size() > 0 && queue == device_queue.get()) || p_swap_chains.size() == 0);

	_update_heap_residency();

	uint32_t size = p_cmd_buffers.size();
	if (size == 0) {
		return OK;
	}

	for (uint32_t i = 0; i < p_wait_sem.size(); i++) {
		Semaphore *sem = (Semaphore *)p_wait_sem[i].id;
		queue->wait(sem->event.get(), sem->value);
	}

	if (size > 1) {
		uint32_t pre_commit_count = size - 1;
		MTL4::CommandBuffer **cmds = ALLOCA_ARRAY(MTL4::CommandBuffer *, pre_commit_count);
		for (uint32_t i = 0; i < pre_commit_count; i++) {
			MDCommandBuffer *cmd_buffer = (MDCommandBuffer *)(p_cmd_buffers[i].id);
			cmd_buffer->commit();
			cmds[i] = cmd_buffer->get_command_buffer();
		}
		queue->commit(cmds, pre_commit_count);
	}

	MDCommandBuffer *cmd_buffer = (MDCommandBuffer *)(p_cmd_buffers[size - 1].id);
	cmd_buffer->commit();

	MTL::Drawable **drawables = ALLOCA_ARRAY(MTL::Drawable *, p_swap_chains.size());
	memset(drawables, 0, sizeof(MTL::Drawable *) * p_swap_chains.size());
	for (uint32_t i = 0; i < p_swap_chains.size(); i++) {
		SwapChain *swap_chain = (SwapChain *)(p_swap_chains[i].id);
		RenderingContextDriverMetal::Surface *metal_surface = (RenderingContextDriverMetal::Surface *)(swap_chain->surface);
		MTL::Drawable *drawable = metal_surface->next_drawable();
		if (drawable) {
			queue->wait(drawable);
			drawables[i] = drawable;
		}
	}

	MTL4::CommandBuffer *cb = cmd_buffer->get_command_buffer();
	queue->commit(&cb, 1);

	for (uint32_t i = 0; i < p_swap_chains.size(); i++) {
		if (drawables[i]) {
			SwapChain *swap_chain = (SwapChain *)(p_swap_chains[i].id);
			RenderingContextDriverMetal::Surface *metal_surface = (RenderingContextDriverMetal::Surface *)(swap_chain->surface);
			queue->signalDrawable(drawables[i]);
			if (metal_surface->vsync_mode != DisplayServerEnums::VSYNC_DISABLED && metal_surface->present_minimum_duration > 0) {
				drawables[i]->presentAfterMinimumDuration(metal_surface->present_minimum_duration);
			} else {
				drawables[i]->present();
			}
		}
	}

	Fence *fence = (Fence *)(p_cmd_fence.id);
	if (fence != nullptr) {
		fence->value++;
		queue->signalEvent(fence->event.get(), fence->value);
	}

	for (uint32_t i = 0; i < p_cmd_sem.size(); i++) {
		Semaphore *sem = (Semaphore *)p_cmd_sem[i].id;
		sem->value++;
		queue->signalEvent(sem->event.get(), sem->value);
	}

	if (p_swap_chains.size() > 0) {
		// Used as a signal that we're presenting, so this is the end of a frame.
		MTL::CaptureScope *scope = device_scope.get();
		scope->endScope();
		scope->beginScope();
	}

	return OK;
}

void RenderingDeviceDriverMetal::command_queue_free(CommandQueueID p_cmd_queue) {
}

#pragma mark - Residency

void RenderingDeviceDriverMetal::add_residency_set_to_main_queue(MTL::ResidencySet *p_set) {
	device_queue->addResidencySet(p_set);
}

void RenderingDeviceDriverMetal::remove_residency_set_to_main_queue(MTL::ResidencySet *p_set) {
	device_queue->removeResidencySet(p_set);
}

void RenderingDeviceDriverMetal::_update_heap_residency() {
	if (allocator->get_heap_generation() == resident_heap_generation) {
		return;
	}
	LocalVector<MTL::Heap *> heaps;
	resident_heap_generation = allocator->get_heaps(heaps);

	MTL::ResidencySet *mrs = main_residency_set.get();
	bool changed = false;
	for (MTL::Heap *heap : heaps) {
		if (resident_heaps.find(heap) < 0) {
			mrs->addAllocation(heap);
			changed = true;
		}
	}
	for (MTL::Heap *heap : resident_heaps) {
		if (heaps.find(heap) < 0) {
			mrs->removeAllocation(heap);
			changed = true;
		}
	}
	if (changed) {
		mrs->commit();
	}
	resident_heaps = heaps;
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

#pragma mark - Timestamp

RDD::QueryPoolID RenderingDeviceDriverMetal::timestamp_query_pool_create(uint32_t p_query_count) {
	NS::SharedPtr<MTL4::CounterHeapDescriptor> desc = NS::TransferPtr(MTL4::CounterHeapDescriptor::alloc()->init());
	desc->setType(MTL4::CounterHeapTypeTimestamp);
	desc->setCount(p_query_count);
	NS::Error *error = nullptr;
	NS::SharedPtr<MTL4::CounterHeap> heap = NS::TransferPtr(device->newCounterHeap(desc.get(), &error));
	ERR_FAIL_COND_V_MSG(error != nullptr, QueryPoolID(), vformat("Failed to create counter heap: %s", String(error->localizedDescription()->utf8String())));
	QueryPool *pool = memnew(QueryPool(heap, p_query_count, device->queryTimestampFrequency()));
	return QueryPoolID(pool);
}

void RenderingDeviceDriverMetal::timestamp_query_pool_free(QueryPoolID p_pool_id) {
	QueryPool *pool = (QueryPool *)(p_pool_id.id);
	memdelete(pool);
}

void RenderingDeviceDriverMetal::timestamp_query_pool_get_results(QueryPoolID p_pool_id, uint32_t p_query_count, uint64_t *r_results) {
	QueryPool *pool = (QueryPool *)(p_pool_id.id);
	pool->get_results(p_query_count, r_results);
}

uint64_t RenderingDeviceDriverMetal::timestamp_query_result_to_time(uint64_t p_result) {
	return p_result; // Already converted to nanoseconds in get_results.
}

void RenderingDeviceDriverMetal::command_timestamp_query_pool_reset(CommandBufferID p_cmd_buffer, QueryPoolID p_pool_id, uint32_t p_query_count) {
	QueryPool *pool = (QueryPool *)(p_pool_id.id);
	pool->invalidate(p_query_count);
}

void RenderingDeviceDriverMetal::command_timestamp_write(CommandBufferID p_cmd_buffer, QueryPoolID p_pool_id, uint32_t p_index) {
	MDCommandBuffer *cmd = (MDCommandBuffer *)(p_cmd_buffer.id);
	QueryPool *pool = (QueryPool *)(p_pool_id.id);
	cmd->timestamp_write(pool, p_index);
}

#pragma mark - Initialization

Error RenderingDeviceDriverMetal::_create_device() {
	device = context_driver->get_metal_device();

	NS::SharedPtr<MTL4::CommandQueueDescriptor> desc = NS::TransferPtr(MTL4::CommandQueueDescriptor::alloc()->init());
	NS::Error *err = nullptr;
	desc->setLabel(MTLSTR("Main Queue"));
	device_queue = NS::TransferPtr(device->newMTL4CommandQueue(desc.get(), &err));
	ERR_FAIL_COND_V_MSG(!device_queue, ERR_CANT_CREATE, "Failed to create main queue");
	desc->setLabel(MTLSTR("Transfer Queue"));
	transfer_queue = NS::TransferPtr(device->newMTL4CommandQueue(desc.get(), &err));
	ERR_FAIL_COND_V_MSG(!transfer_queue, ERR_CANT_CREATE, "Failed to create transfer queue");

	device_scope = NS::TransferPtr(MTL::CaptureManager::sharedCaptureManager()->newCaptureScope(device));
	device_scope->setLabel(MTLSTR("Godot Frame"));
	device_scope->beginScope(); // Allow Xcode to capture the first frame, if desired.

	NS::SharedPtr<MTL::ResidencySetDescriptor> rs_desc = NS::TransferPtr(MTL::ResidencySetDescriptor::alloc()->init());
	rs_desc->setInitialCapacity(10);
	rs_desc->setLabel(MTLSTR("Main Residency Set"));
	NS::Error *error = nullptr;
	main_residency_set = NS::TransferPtr(device->newResidencySet(rs_desc.get(), &error));
	CRASH_COND_MSG(error != nullptr, vformat("Failed to create residency set: %s", String(error->localizedDescription()->utf8String())));

	device_queue->addResidencySet(main_residency_set.get());
	transfer_queue->addResidencySet(main_residency_set.get());

	return OK;
}

void RenderingDeviceDriverMetal::_resolve_sync_mode() {
	// Metal 4's driver path requires explicit synchronization rather than native hazard tracking.
	if (sync_mode == HazardTracking) {
		WARN_PRINT("Metal 4: Hazard tracking is not supported for Metal 4. Falling back to barriers.");
		sync_mode = Barriers;
	}
	base_hazard_tracking = MTL::ResourceHazardTrackingModeUntracked;
}

Error RenderingDeviceDriverMetal::initialize(uint32_t p_device_index, uint32_t p_frame_count) {
	// Call base class shared initialization.
	Error err = _initialize(p_device_index, p_frame_count);
	ERR_FAIL_COND_V(err, err);

	{
		NS::SharedPtr<MTL4::CompilerDescriptor> desc = NS::TransferPtr(MTL4::CompilerDescriptor::alloc()->init());
		compiler = NS::TransferPtr(device->newCompiler(desc.get(), nullptr));
	}

	return OK;
}

} //namespace API_AVAILABLE(macos(26.0),ios(26.0),tvos(26.0),visionos(26.0))MTL4

#endif // METAL4_ENABLED
