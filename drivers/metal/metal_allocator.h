/**************************************************************************/
/*  metal_allocator.h                                                     */
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

#include "core/os/spin_lock.h"
#include "core/templates/local_vector.h"
#include "core/templates/safe_refcount.h"

#include <thirdparty/offset_allocator/offsetAllocator.hpp>

#include <Metal/Metal.hpp>

/// Opaque allocation handle. POOL entries reference a pool block plus the
/// OffsetAllocator range (in 256-byte units); DEDICATED entries own a
/// single-resource placement heap. INVALID means there is nothing to return
/// to the allocator (passthrough allocations, committed resources, texture
/// views, external imports).
struct MetalAllocation {
	enum class Kind : uint8_t {
		INVALID,
		POOL,
		DEDICATED,
	};

	Kind kind = Kind::INVALID;
	uint8_t pool = 0;
	uint32_t block = 0; // POOL only.
	OffsetAllocator::Allocation alloc; // POOL only.
	MTL::Heap *heap = nullptr; // DEDICATED only.

	_FORCE_INLINE_ bool is_valid() const { return kind != Kind::INVALID; }
	_FORCE_INLINE_ void invalidate() {
		kind = Kind::INVALID;
		heap = nullptr;
	}
};

struct MetalBuffer {
	NS::SharedPtr<MTL::Buffer> buffer;
	MetalAllocation allocation;
};

struct MetalTexture {
	NS::SharedPtr<MTL::Texture> texture;
	MetalAllocation allocation;
};

#ifdef DEBUG_ENABLED
struct MetalAllocatorStats {
	/// Live statistics for the associated pool.
	struct Pool {
		uint64_t reserved_bytes = 0;
		uint64_t used_bytes = 0;
		// The number of blocks.
		uint32_t block_count = 0;
		// Total number of allocations.
		uint32_t allocation_count = 0;
		// Total number of dedicated heaps.
		uint32_t dedicated_count = 0;
		// Total size of dedicated heaps.
		uint64_t dedicated_bytes = 0;
	};
	Pool pools[3];
};
#endif

/// Abstract allocation interface for the Metal driver. The driver holds a
/// single instance selected by sync mode.
class API_AVAILABLE(macos(11.0), ios(14.0), tvos(14.0)) MetalAllocator {
public:
	static MetalAllocator *create(MTL::Device *p_device, bool p_use_heaps, uint32_t p_frame_count);
	virtual ~MetalAllocator() = default;

	void next_frame(uint64_t p_frames_drawn) { frames_drawn.set(p_frames_drawn); }

	virtual MetalBuffer new_buffer(NS::UInteger p_length, MTL::ResourceOptions p_options) = 0;
	virtual MetalTexture new_texture(const MTL::TextureDescriptor *p_desc) = 0;
	virtual void free_buffer(MetalBuffer &p_buffer) = 0;
	virtual void free_texture(MetalTexture &p_texture) = 0;

	/// Appends all heaps with live allocations to r_heaps and returns the
	/// generation the snapshot corresponds to. The generation increments
	/// whenever a heap enters or leaves that set; callers cache their heap
	/// arrays against it.
	virtual uint64_t get_heaps(LocalVector<MTL::Heap *> &r_heaps) = 0;
	virtual uint64_t get_heap_generation() const = 0;

#ifdef DEBUG_ENABLED
	virtual void get_stats(MetalAllocatorStats &r_stats) = 0;
#endif

protected:
	SafeNumeric<uint64_t> frames_drawn;
};

/// Passthrough: forwards to MTL::Device, returns empty allocation handles.
/// Used in hazard tracking mode. Behavior identical to the pre-allocator
/// driver.
class API_AVAILABLE(macos(11.0), ios(14.0), tvos(14.0)) MetalDeviceAllocator final : public MetalAllocator {
	MTL::Device *device = nullptr;

public:
	explicit MetalDeviceAllocator(MTL::Device *p_device) :
			device(p_device) {}

	MetalBuffer new_buffer(NS::UInteger p_length, MTL::ResourceOptions p_options) override;
	MetalTexture new_texture(const MTL::TextureDescriptor *p_desc) override;
	void free_buffer(MetalBuffer &p_buffer) override;
	void free_texture(MetalTexture &p_texture) override;
	uint64_t get_heaps(LocalVector<MTL::Heap *> &r_heaps) override;
	uint64_t get_heap_generation() const override;
#ifdef DEBUG_ENABLED
	void get_stats(MetalAllocatorStats &r_stats) override;
#endif
};

/// Placement-heap suballocator for barriers mode. Three pools keyed by
/// storage mode (+ CPU cache mode for the write-combined pool); blocks are
/// placement heaps with OffsetAllocator metadata in 256-byte units;
/// resources larger than half a pool's preferred block size get a dedicated
/// single-resource heap.
class API_AVAILABLE(macos(11.0), ios(14.0), tvos(14.0)) MetalHeapAllocator final : public MetalAllocator {
public:
	enum PoolIndex : uint32_t {
		POOL_PRIVATE = 0, // StorageModePrivate; memoryless textures also land here.
		POOL_SHARED = 1, // StorageModeShared, default CPU cache.
		POOL_SHARED_WC = 2, // StorageModeShared, write-combined CPU cache.
		POOL_COUNT = 3,
	};

	/// The smallest allocation the allocator works in, in bytes. OffsetAllocator
	/// hands out ranges in whole units, so every offset is a multiple of 256,
	/// which satisfies the alignment of every resource on Apple silicon
	/// (buffers report 256, textures 128) with no per-allocation padding.
	/// Should a format ever report more than 256, _pool_allocate defends
	/// itself by overallocating and rounding the offset up.
	static constexpr uint64_t UNIT = 256;
	/// How many allocations one block can hold at once. OffsetAllocator
	/// asks for this up front (its default is 128K) because it allocates
	/// all of its bookkeeping in one go, at about 32 bytes per entry.
	///
	/// 16K entries costs about 512 KiB of bookkeeping per block instead of
	/// 4 MiB at the default.
	static constexpr uint32_t MAX_ALLOCS_PER_BLOCK = 16384;

private:
	struct Block {
		NS::SharedPtr<MTL::Heap> heap;
		OffsetAllocator::Allocator *metadata = nullptr;
		// frames_drawn when live last reached 0. Empty blocks are reused by
		// _pool_allocate and reclaimed by a later free once frame_count frames
		// have passed, so no in-flight command buffer still lists the heap.
		uint64_t empty_since = 0;
		// The number of live allocations in this block.
		uint32_t live = 0;
	};

	struct RetiredHeap {
		NS::SharedPtr<MTL::Heap> heap;
		uint64_t retired_frame = 0;
	};

	struct Pool {
		SpinLock mutex;
		LocalVector<Block> blocks;
		LocalVector<MTL::Heap *> dedicated_heaps; // Owned (+1) refs, for get_heaps enumeration.
		LocalVector<RetiredHeap> retired_dedicated; // Freed, released by a later free.
		uint32_t empty_blocks = 0; // Blocks with live == 0, so _reclaim_heaps can skip the scan.
#ifdef DEBUG_ENABLED
		MetalAllocatorStats::Pool stats;
#endif
	};

	MTL::Device *device = nullptr;
	Pool pools[POOL_COUNT];
	uint32_t frame_count = 1;
	/// Incremented each time a heap is created or destroyed so clients know when to update
	/// their own state and refresh residency.
	SafeNumeric<uint64_t> generation = SafeNumeric<uint64_t>(1);

	static uint64_t _preferred_block_size(uint32_t p_pool);
	static uint32_t _pool_for_options(MTL::ResourceOptions p_options);
	static uint32_t _pool_for_texture(const MTL::TextureDescriptor *p_desc);
	NS::SharedPtr<MTL::Heap> _create_heap(uint32_t p_pool, uint64_t p_size);
	/// Allocates p_size bytes aligned to p_align from pool p_pool.
	/// On success fills r_allocation (Kind::POOL), r_heap and r_offset.
	bool _pool_allocate(uint32_t p_pool, uint64_t p_size, uint64_t p_align, MetalAllocation &r_allocation, MTL::Heap *&r_heap, uint64_t &r_offset);
	/// Creates a dedicated single-resource heap (Kind::DEDICATED).
	bool _dedicated_allocate(uint32_t p_pool, uint64_t p_size, MetalAllocation &r_allocation, MTL::Heap *&r_heap);
	void _free_allocation(MetalAllocation &p_allocation);
	/// Releases p_pool's empty blocks and retired dedicated heaps that have
	/// aged at least p_min_age frames. Takes the pool lock per heap and
	/// releases outside it. Call without the lock held.
	void _reclaim_heaps(Pool &p_pool, uint64_t p_min_age);

public:
	MetalHeapAllocator(MTL::Device *p_device, uint32_t p_frame_count) :
			device(p_device), frame_count(p_frame_count) {}
	~MetalHeapAllocator() override;

	MetalBuffer new_buffer(NS::UInteger p_length, MTL::ResourceOptions p_options) override;
	MetalTexture new_texture(const MTL::TextureDescriptor *p_desc) override;
	void free_buffer(MetalBuffer &p_buffer) override;
	void free_texture(MetalTexture &p_texture) override;
	uint64_t get_heaps(LocalVector<MTL::Heap *> &r_heaps) override;
	uint64_t get_heap_generation() const override;
#ifdef DEBUG_ENABLED
	void get_stats(MetalAllocatorStats &r_stats) override;
#endif
};
