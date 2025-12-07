/**************************************************************************/
/*  metal_objects_shared.h                                                */
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

#import "metal_device_properties.h"
#import "metal_utils.h"
#import "pixel_formats.h"

using RDC = RenderingDeviceCommons;

// These types can be used in Vector and other containers that use
// pointer operations not supported by ARC.
namespace MTL {
#define MTL_CLASS(name)                               \
	class name {                                      \
	public:                                           \
		name(id<MTL##name> obj = nil) : m_obj(obj) {} \
		operator id<MTL##name>() const {              \
			return m_obj;                             \
		}                                             \
		id<MTL##name> m_obj;                          \
	};

MTL_CLASS(Texture)

} //namespace MTL

typedef id<MTLResource> __unsafe_unretained MTLResourceUnsafe;

template <>
struct HashMapHasherDefaultImpl<MTLResourceUnsafe> {
	static _FORCE_INLINE_ uint32_t hash(const MTLResourceUnsafe p_pointer) { return hash_one_uint64((uint64_t)p_pointer); }
};

enum ShaderStageUsage : uint32_t {
	None = 0,
	Vertex = RDD::SHADER_STAGE_VERTEX_BIT,
	Fragment = RDD::SHADER_STAGE_FRAGMENT_BIT,
	TesselationControl = RDD::SHADER_STAGE_TESSELATION_CONTROL_BIT,
	TesselationEvaluation = RDD::SHADER_STAGE_TESSELATION_EVALUATION_BIT,
	Compute = RDD::SHADER_STAGE_COMPUTE_BIT,
};

_FORCE_INLINE_ ShaderStageUsage &operator|=(ShaderStageUsage &p_a, int p_b) {
	p_a = ShaderStageUsage(uint32_t(p_a) | uint32_t(p_b));
	return p_a;
}

struct ClearAttKey {
	const static uint32_t COLOR_COUNT = MAX_COLOR_ATTACHMENT_COUNT;
	const static uint32_t DEPTH_INDEX = COLOR_COUNT;
	const static uint32_t STENCIL_INDEX = DEPTH_INDEX + 1;
	const static uint32_t ATTACHMENT_COUNT = STENCIL_INDEX + 1;

	enum Flags : uint16_t {
		CLEAR_FLAGS_NONE = 0,
		CLEAR_FLAGS_LAYERED = 1 << 0,
	};

	Flags flags = CLEAR_FLAGS_NONE;
	uint16_t sample_count = 0;
	uint16_t pixel_formats[ATTACHMENT_COUNT] = { 0 };

	_FORCE_INLINE_ void set_color_format(uint32_t p_idx, MTLPixelFormat p_fmt) { pixel_formats[p_idx] = p_fmt; }
	_FORCE_INLINE_ void set_depth_format(MTLPixelFormat p_fmt) { pixel_formats[DEPTH_INDEX] = p_fmt; }
	_FORCE_INLINE_ void set_stencil_format(MTLPixelFormat p_fmt) { pixel_formats[STENCIL_INDEX] = p_fmt; }
	_FORCE_INLINE_ MTLPixelFormat depth_format() const { return (MTLPixelFormat)pixel_formats[DEPTH_INDEX]; }
	_FORCE_INLINE_ MTLPixelFormat stencil_format() const { return (MTLPixelFormat)pixel_formats[STENCIL_INDEX]; }
	_FORCE_INLINE_ void enable_layered_rendering() { flags::set(flags, CLEAR_FLAGS_LAYERED); }

	_FORCE_INLINE_ bool is_enabled(uint32_t p_idx) const { return pixel_formats[p_idx] != 0; }
	_FORCE_INLINE_ bool is_depth_enabled() const { return pixel_formats[DEPTH_INDEX] != 0; }
	_FORCE_INLINE_ bool is_stencil_enabled() const { return pixel_formats[STENCIL_INDEX] != 0; }
	_FORCE_INLINE_ bool is_layered_rendering_enabled() const { return flags::any(flags, CLEAR_FLAGS_LAYERED); }

	_FORCE_INLINE_ bool operator==(const ClearAttKey &p_rhs) const {
		return memcmp(this, &p_rhs, sizeof(ClearAttKey)) == 0;
	}

	uint32_t hash() const {
		uint32_t h = hash_murmur3_one_32(flags);
		h = hash_murmur3_one_32(sample_count, h);
		h = hash_murmur3_buffer(pixel_formats, ATTACHMENT_COUNT * sizeof(pixel_formats[0]), h);
		return hash_fmix32(h);
	}
};

#pragma mark - Ring Buffer

/// A ring buffer backed by MTLBuffer instances for transient GPU allocations.
/// Allocations are 16-byte aligned with a minimum size of 16 bytes.
/// When the current buffer is exhausted, a new buffer is allocated.
class API_AVAILABLE(macos(11.0), ios(14.0), tvos(14.0)) MDRingBuffer {
public:
	static constexpr uint32_t DEFAULT_BUFFER_SIZE = 512 * 1024;
	static constexpr uint32_t MIN_BLOCK_SIZE = 16;
	static constexpr uint32_t ALIGNMENT = 16;

	struct Allocation {
		void *ptr = nullptr;
		id<MTLBuffer> buffer = nil;
		uint64_t gpu_address = 0;
		uint32_t offset = 0;

		_FORCE_INLINE_ bool is_valid() const { return ptr != nullptr; }
	};

private:
	id<MTLDevice> device = nil;
	LocalVector<id<MTLBuffer>> buffers;
	LocalVector<uint32_t> heads;
	uint32_t current_segment = 0;
	uint32_t buffer_size = DEFAULT_BUFFER_SIZE;
	bool changed = false;

	_FORCE_INLINE_ uint32_t alloc_segment() {
		id<MTLBuffer> buffer = [device newBufferWithLength:buffer_size
												   options:MTLResourceStorageModeShared | MTLResourceHazardTrackingModeUntracked];
		buffers.push_back(buffer);
		heads.push_back(0);
		changed = true;

		return buffers.size() - 1;
	}

public:
	MDRingBuffer() = default;

	MDRingBuffer(id<MTLDevice> p_device, uint32_t p_buffer_size = DEFAULT_BUFFER_SIZE) :
			device(p_device), buffer_size(p_buffer_size) {}

	~MDRingBuffer() {
		for (uint32_t i = 0; i < buffers.size(); i++) {
			buffers[i] = nil;
		}
	}

	/// Allocates a block of memory from the ring buffer.
	/// Returns an Allocation with the pointer, buffer, and offset.
	_FORCE_INLINE_ Allocation allocate(uint32_t p_size) {
		p_size = MAX(p_size, MIN_BLOCK_SIZE);
		p_size = (p_size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);

		if (buffers.is_empty()) {
			alloc_segment();
		}

		uint32_t aligned_head = (heads[current_segment] + ALIGNMENT - 1) & ~(ALIGNMENT - 1);

		if (aligned_head + p_size > buffer_size) {
			// Current segment exhausted, try to find one with space or allocate new.
			bool found = false;
			for (uint32_t i = 0; i < buffers.size(); i++) {
				uint32_t ah = (heads[i] + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
				if (ah + p_size <= buffer_size) {
					current_segment = i;
					aligned_head = ah;
					found = true;
					break;
				}
			}

			if (!found) {
				current_segment = alloc_segment();
				aligned_head = 0;
			}
		}

		id<MTLBuffer> buffer = buffers[current_segment];
		Allocation alloc;
		alloc.buffer = buffer;
		alloc.offset = aligned_head;
		alloc.ptr = static_cast<uint8_t *>([buffer contents]) + aligned_head;
		if (@available(macOS 13.0, iOS 16.0, tvOS 16.0, *)) {
			alloc.gpu_address = buffer.gpuAddress + aligned_head;
		}
		heads[current_segment] = aligned_head + p_size;

		return alloc;
	}

	/// Resets all segments for reuse. Call at frame boundaries when GPU work is complete.
	_FORCE_INLINE_ void reset() {
		for (uint32_t &head : heads) {
			head = 0;
		}
		current_segment = 0;
	}

	/// Returns true if buffers were added or removed since last clear_changed().
	_FORCE_INLINE_ bool is_changed() const { return changed; }

	/// Clears the changed flag.
	_FORCE_INLINE_ void clear_changed() { changed = false; }

	/// Returns a Span of all backing buffers.
	_FORCE_INLINE_ Span<const id<MTLBuffer> __unsafe_unretained> get_buffers() const {
		return Span<const id<MTLBuffer> __unsafe_unretained>(buffers.ptr(), buffers.size());
	}

	/// Returns the number of buffer segments currently allocated.
	_FORCE_INLINE_ uint32_t get_segment_count() const {
		return buffers.size();
	}
};

#pragma mark - Resource Factory

class API_AVAILABLE(macos(11.0), ios(14.0), tvos(14.0)) MDResourceFactory {
private:
	id<MTLDevice> device;
	PixelFormats &pixel_formats;
	uint32_t max_buffer_count;

	id<MTLFunction> new_func(NSString *p_source, NSString *p_name, NSError **p_error);
	id<MTLFunction> new_clear_vert_func(ClearAttKey &p_key);
	id<MTLFunction> new_clear_frag_func(ClearAttKey &p_key);
	NSString *get_format_type_string(MTLPixelFormat p_fmt);

	_FORCE_INLINE_ uint32_t get_vertex_buffer_index(uint32_t p_binding) {
		return (max_buffer_count - 1) - p_binding;
	}

public:
	id<MTLRenderPipelineState> new_clear_pipeline_state(ClearAttKey &p_key, NSError **p_error);
	id<MTLRenderPipelineState> new_empty_draw_pipeline_state(ClearAttKey &p_key, NSError **p_error);
	id<MTLDepthStencilState> new_depth_stencil_state(bool p_use_depth, bool p_use_stencil);

	MDResourceFactory(id<MTLDevice> p_device, PixelFormats &p_pixel_formats, uint32_t p_max_buffer_count) :
			device(p_device), pixel_formats(p_pixel_formats), max_buffer_count(p_max_buffer_count) {}
	~MDResourceFactory() = default;
};

class API_AVAILABLE(macos(11.0), ios(14.0), tvos(14.0)) MDResourceCache {
private:
	typedef HashMap<ClearAttKey, id<MTLRenderPipelineState>> HashMap;
	std::unique_ptr<MDResourceFactory> resource_factory;
	HashMap clear_states;
	HashMap empty_draw_states;

	struct {
		id<MTLDepthStencilState> all;
		id<MTLDepthStencilState> depth_only;
		id<MTLDepthStencilState> stencil_only;
		id<MTLDepthStencilState> none;
	} clear_depth_stencil_state;

public:
	id<MTLRenderPipelineState> get_clear_render_pipeline_state(ClearAttKey &p_key, NSError **p_error);
	id<MTLRenderPipelineState> get_empty_draw_pipeline_state(ClearAttKey &p_key, NSError **p_error);
	id<MTLDepthStencilState> get_depth_stencil_state(bool p_use_depth, bool p_use_stencil);

	explicit MDResourceCache(id<MTLDevice> p_device, PixelFormats &p_pixel_formats, uint32_t p_max_buffer_count) :
			resource_factory(new MDResourceFactory(p_device, p_pixel_formats, p_max_buffer_count)) {}
	~MDResourceCache() = default;
};

/**
 * Returns an index that can be used to map a shader stage to an index in a fixed-size array that is used for
 * a single pipeline type.
 */
_FORCE_INLINE_ static uint32_t to_index(RDD::ShaderStage p_s) {
	switch (p_s) {
		case RenderingDeviceCommons::SHADER_STAGE_VERTEX:
		case RenderingDeviceCommons::SHADER_STAGE_TESSELATION_CONTROL:
		case RenderingDeviceCommons::SHADER_STAGE_TESSELATION_EVALUATION:
		case RenderingDeviceCommons::SHADER_STAGE_COMPUTE:
		default:
			return 0;
		case RenderingDeviceCommons::SHADER_STAGE_FRAGMENT:
			return 1;
	}
}

class API_AVAILABLE(macos(11.0), ios(14.0), tvos(14.0)) MDFrameBuffer {
	Vector<MTL::Texture> textures;

public:
	Size2i size;
	MDFrameBuffer(Vector<MTL::Texture> p_textures, Size2i p_size) :
			textures(p_textures), size(p_size) {}
	MDFrameBuffer() {}

	/// Returns the texture at the given index.
	_ALWAYS_INLINE_ MTL::Texture get_texture(uint32_t p_idx) const {
		return textures[p_idx];
	}

	/// Returns true if the texture at the given index is not nil.
	_ALWAYS_INLINE_ bool has_texture(uint32_t p_idx) const {
		return textures[p_idx] != nil;
	}

	/// Set the texture at the given index.
	_ALWAYS_INLINE_ void set_texture(uint32_t p_idx, MTL::Texture p_texture) {
		textures.write[p_idx] = p_texture;
	}

	/// Unset or nil the texture at the given index.
	_ALWAYS_INLINE_ void unset_texture(uint32_t p_idx) {
		textures.write[p_idx] = nil;
	}

	/// Resizes buffers to the specified size.
	_ALWAYS_INLINE_ void set_texture_count(uint32_t p_size) {
		textures.resize(p_size);
	}

	virtual ~MDFrameBuffer() = default;
};

template <>
struct HashMapComparatorDefault<RDD::ShaderID> {
	static bool compare(const RDD::ShaderID &p_lhs, const RDD::ShaderID &p_rhs) {
		return p_lhs.id == p_rhs.id;
	}
};

template <>
struct HashMapComparatorDefault<RDD::BufferID> {
	static bool compare(const RDD::BufferID &p_lhs, const RDD::BufferID &p_rhs) {
		return p_lhs.id == p_rhs.id;
	}
};

template <>
struct HashMapComparatorDefault<RDD::TextureID> {
	static bool compare(const RDD::TextureID &p_lhs, const RDD::TextureID &p_rhs) {
		return p_lhs.id == p_rhs.id;
	}
};

template <>
struct HashMapHasherDefaultImpl<RDD::BufferID> {
	static _FORCE_INLINE_ uint32_t hash(const RDD::BufferID &p_value) {
		return HashMapHasherDefaultImpl<uint64_t>::hash(p_value.id);
	}
};

template <>
struct HashMapHasherDefaultImpl<RDD::TextureID> {
	static _FORCE_INLINE_ uint32_t hash(const RDD::TextureID &p_value) {
		return HashMapHasherDefaultImpl<uint64_t>::hash(p_value.id);
	}
};

// These functions are used to convert between Objective-C objects and
// the RIDs used by Godot, respecting automatic reference counting.
namespace rid {

// Converts an Objective-C object to a pointer, and incrementing the
// reference count.
_FORCE_INLINE_ void *owned(id p_id) {
	return (__bridge_retained void *)p_id;
}

#define MAKE_ID(FROM, TO)                \
	_FORCE_INLINE_ TO make(FROM p_obj) { \
		return TO(owned(p_obj));         \
	}

// These are shared for Metal and Metal 4 drivers

MAKE_ID(id<MTLTexture>, RDD::TextureID)
MAKE_ID(id<MTLBuffer>, RDD::BufferID)
MAKE_ID(id<MTLSamplerState>, RDD::SamplerID)
MAKE_ID(MTLVertexDescriptor *, RDD::VertexFormatID)

#undef MAKE_ID

// Converts a pointer to an Objective-C object without changing the reference count.
_FORCE_INLINE_ auto get(RDD::ID p_id) {
	return (p_id.id) ? (__bridge ::id)(void *)p_id.id : nil;
}

// Converts a pointer to an Objective-C object, and decrements the reference count.
_FORCE_INLINE_ auto release(RDD::ID p_id) {
	return (__bridge_transfer ::id)(void *)p_id.id;
}

} // namespace rid
