/**************************************************************************/
/*  rendering_device_driver_metal4.h                                      */
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

#include "metal4_objects.h"
#include "rendering_device_driver_metal.h"

#include <Metal/Metal.hpp>

namespace MTL4 {

class API_AVAILABLE(macos(26.0), ios(26.0), tvos(26.0), visionos(26.0)) RenderingDeviceDriverMetal final : public ::RenderingDeviceDriverMetal {
	friend struct ShaderCacheEntry;
	friend class MDCommandBuffer;
	friend class MDUniformSet;

#pragma mark - Generic

	NS::SharedPtr<MTL4::CommandQueue> device_queue;
	NS::SharedPtr<MTL4::CommandQueue> transfer_queue;
	NS::SharedPtr<MTL4::Compiler> compiler;

	/// Generation of `allocator`'s heap set last synced to `main_residency_set`.
	uint64_t resident_heap_generation = 0;
	/// Heaps currently added to `main_residency_set`, as of `resident_heap_generation`.
	LocalVector<MTL::Heap *> resident_heaps;
	/// Syncs `main_residency_set` with the allocator's current heap set, if the
	/// allocator's heap generation has changed since the last sync.
	void _update_heap_residency();

protected:
	Error _create_device() override;
	void _resolve_sync_mode() override;
	MTL::CommandQueue *get_command_queue() const override { return reinterpret_cast<MTL::CommandQueue *>(device_queue.get()); }
	void add_residency_set_to_main_queue(MTL::ResidencySet *p_set) override;
	void remove_residency_set_to_main_queue(MTL::ResidencySet *p_set) override;

public:
	Error initialize(uint32_t p_device_index, uint32_t p_frame_count) override;

	MTL4::Compiler *get_compiler() const { return compiler.get(); }

#pragma mark - Fences

private:
	struct Fence {
		NS::SharedPtr<MTL::SharedEvent> event;
		uint64_t value = 0;
		Fence(NS::SharedPtr<MTL::SharedEvent> p_event) :
				event(p_event) {}
	};

public:
	FenceID fence_create() override;
	Error fence_wait(FenceID p_fence) override;
	void fence_free(FenceID p_fence) override;

#pragma mark - Semaphores

private:
	struct Semaphore {
		NS::SharedPtr<MTL::Event> event;
		uint64_t value = 0;
		Semaphore(NS::SharedPtr<MTL::Event> p_event) :
				event(p_event) {}
	};

public:
	SemaphoreID semaphore_create() override;
	void semaphore_free(SemaphoreID p_semaphore) override;

#pragma mark - Commands
	// ----- QUEUE -----
public:
	CommandQueueID command_queue_create(CommandQueueFamilyID p_cmd_queue_family, bool p_identify_as_main_queue = false) override;
	Error command_queue_execute_and_present(CommandQueueID p_cmd_queue, VectorView<SemaphoreID> p_wait_semaphores, VectorView<CommandBufferID> p_cmd_buffers, VectorView<SemaphoreID> p_cmd_semaphores, FenceID p_cmd_fence, VectorView<SwapChainID> p_swap_chains) override;
	void command_queue_free(CommandQueueID p_cmd_queue) override;

	// ----- POOL -----

private:
	LocalVector<MD4CommandPool *> command_pools;

public:
	CommandPoolID command_pool_create(CommandQueueFamilyID p_cmd_queue_family, CommandBufferType p_cmd_buffer_type) override;
	bool command_pool_reset(CommandPoolID p_cmd_pool) override;
	void command_pool_free(CommandPoolID p_cmd_pool) override;

	// ----- BUFFER -----

private:
	// Used to maintain references.
	Vector<MDCommandBuffer *> command_buffers;

public:
	CommandBufferID command_buffer_create(CommandPoolID p_cmd_pool) override;

#pragma mark - Timestamp

	QueryPoolID timestamp_query_pool_create(uint32_t p_query_count) override;
	void timestamp_query_pool_free(QueryPoolID p_pool_id) override;
	void timestamp_query_pool_get_results(QueryPoolID p_pool_id, uint32_t p_query_count, uint64_t *r_results) override;
	uint64_t timestamp_query_result_to_time(uint64_t p_result) override;
	void command_timestamp_query_pool_reset(CommandBufferID p_cmd_buffer, QueryPoolID p_pool_id, uint32_t p_query_count) override;
	void command_timestamp_write(CommandBufferID p_cmd_buffer, QueryPoolID p_pool_id, uint32_t p_index) override;

#pragma mark - Miscellaneous

	String get_api_name() const override { return "Metal4"; }

	RenderingDeviceDriverMetal(RenderingContextDriverMetal *p_context_driver);
	~RenderingDeviceDriverMetal();
};

} // namespace MTL4
