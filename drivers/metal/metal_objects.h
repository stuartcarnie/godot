/**************************************************************************/
/*  metal_objects.h                                                       */
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

#ifndef GODOT_METAL_OBJECTS_H
#define GODOT_METAL_OBJECTS_H

#import "metal_device_properties.h"
#import "servers/rendering/rendering_device_driver.h"
#import "utils.h"

#import "spirv.hpp"
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <simd/simd.h>

// These types can be used in Vector and other containers that use
// pointer operations not supported by ARC.
namespace MTL {
#define MTL_CLASS(name)                                  \
	class name {                                         \
	public:                                              \
		name(id<MTL##name> obj = nil) : m_obj(obj) {}    \
		operator id<MTL##name>() const { return m_obj; } \
		id<MTL##name> m_obj;                             \
	};

MTL_CLASS(Texture)

} //namespace MTL

enum ShaderStageUsage : uint8_t {
	None = 0,
	Vertex = RDD::SHADER_STAGE_VERTEX_BIT,
	Fragment = RDD::SHADER_STAGE_FRAGMENT_BIT,
	TesselationControl = RDD::SHADER_STAGE_TESSELATION_CONTROL_BIT,
	TesselationEvaluation = RDD::SHADER_STAGE_TESSELATION_EVALUATION_BIT,
	Compute = RDD::SHADER_STAGE_COMPUTE_BIT,
};

_FORCE_INLINE_ ShaderStageUsage &operator|=(ShaderStageUsage &a, int b) {
	a = ShaderStageUsage(uint8_t(a) | uint8_t(b));
	return a;
}

enum class MDCommandBufferStateType {
	None,
	Render,
	Compute,
	Blit,
};

enum class MDPipelineType {
	None,
	Render,
	Compute,
};

class MDRenderPass;
class MDPipeline;
class MDRenderPipeline;
class MDComputePipeline;
class MDFrameBuffer;
class MDQueryPool;
class MetalContext;
class MDUniformSet;
class MDShader;

#pragma mark - Resource Factory

struct ClearAttKey {
	const static uint32_t COLOR_COUNT = MAX_COLOR_ATTACHMENT_COUNT;
	const static uint32_t DEPTH_INDEX = COLOR_COUNT;
	const static uint32_t STENCIL_INDEX = DEPTH_INDEX + 1;
	const static uint32_t ATTACHMENT_COUNT = STENCIL_INDEX + 1;

	uint16_t sample_count = 0;
	uint16_t pixel_formats[ATTACHMENT_COUNT] = { 0 };

	_FORCE_INLINE_ void set_color_format(uint32_t idx, MTLPixelFormat fmt) { pixel_formats[idx] = fmt; }
	_FORCE_INLINE_ void set_depth_format(MTLPixelFormat fmt) { pixel_formats[DEPTH_INDEX] = fmt; }
	_FORCE_INLINE_ void set_stencil_format(MTLPixelFormat fmt) { pixel_formats[STENCIL_INDEX] = fmt; }
	_FORCE_INLINE_ MTLPixelFormat depth_format() const { return (MTLPixelFormat)pixel_formats[DEPTH_INDEX]; }
	_FORCE_INLINE_ MTLPixelFormat stencil_format() const { return (MTLPixelFormat)pixel_formats[STENCIL_INDEX]; }

	_FORCE_INLINE_ bool is_enabled(uint32_t idx) const { return pixel_formats[idx] != 0; }
	_FORCE_INLINE_ bool is_depth_enabled() const { return pixel_formats[DEPTH_INDEX] != 0; }
	_FORCE_INLINE_ bool is_stencil_enabled() const { return pixel_formats[STENCIL_INDEX] != 0; }

	_FORCE_INLINE_ bool operator==(const ClearAttKey &rhs) const { return mvkAreEqual(this, &rhs); }

	[[nodiscard]] uint32_t hash() const {
		uint32_t h = hash_murmur3_one_32(sample_count);
		h = hash_murmur3_buffer(pixel_formats, ATTACHMENT_COUNT * sizeof(pixel_formats[0]), h);
		return h;
	}
};

class MDResourceFactory {
private:
	id<MTLDevice> device;
	MetalContext *context;

	id<MTLFunction> new_func(NSString *p_source, NSString *p_name, NSError **p_error);
	id<MTLFunction> new_clear_vert_func(ClearAttKey &p_key);
	id<MTLFunction> new_clear_frag_func(ClearAttKey &p_key);
	NSString *get_format_type_string(MTLPixelFormat p_fmt);

public:
	id<MTLRenderPipelineState> new_clear_pipeline_state(ClearAttKey &p_key, NSError **p_error);
	id<MTLDepthStencilState> new_depth_stencil_state(bool p_use_depth, bool p_use_stencil);

	MDResourceFactory(id<MTLDevice> p_device, MetalContext *p_context) :
			device(p_device), context(p_context) {}
	~MDResourceFactory() = default;
};

class MDResourceCache {
private:
	std::unique_ptr<MDResourceFactory> resource_factory;
	HashMap<ClearAttKey, id<MTLRenderPipelineState>, HashableHasher<ClearAttKey>> clear_states;

	struct {
		id<MTLDepthStencilState> all;
		id<MTLDepthStencilState> depth_only;
		id<MTLDepthStencilState> stencil_only;
		id<MTLDepthStencilState> none;
	} clear_depth_stencil_state;

public:
	id<MTLRenderPipelineState> get_clear_render_pipeline_state(ClearAttKey &p_key, NSError **p_error);
	id<MTLDepthStencilState> get_depth_stencil_state(bool p_use_depth, bool p_use_stencil);

	explicit MDResourceCache(id<MTLDevice> p_device, MetalContext *p_context) :
			resource_factory(new MDResourceFactory(p_device, p_context)) {}
	~MDResourceCache() = default;
};

class MDCommandBuffer {
private:
	MetalContext *context = nullptr;
	id<MTLCommandQueue> queue = nil;
	id<MTLCommandBuffer> commandBuffer = nil;

	void end_compute_dispatch();
	void end_blit();

#pragma mark - Render

	void render_bind_uniform_sets();

	static void populate_vertices(simd::float4 *p_vertices, Size2i p_fb_size, VectorView<RDD::AttachmentClearRect> p_rects);
	static uint32_t populate_vertices(simd::float4 *p_vertices, uint32_t p_index, RDD::AttachmentClearRect const &p_rect, Size2i p_fb_size);

public:
	MDCommandBufferStateType type = MDCommandBufferStateType::None;

	// state
	struct RenderState {
		MDRenderPass *pass = nullptr;
		MDFrameBuffer *frameBuffer = nullptr;
		MDRenderPipeline *pipeline = nullptr;
		uint32_t current_subpass = UINT32_MAX;
		Rect2i render_area = {};
		bool is_rendering_entire_area = false;
		id<MTLRenderCommandEncoder> encoder = nil;
		id<MTLBuffer> index_buffer = nil;
		MTLIndexType index_type = MTLIndexTypeUInt16;
		// clang-format off
		enum DirtyFlag: uint8_t {
			DIRTY_NONE     = 0b0000,
			DIRTY_PIPELINE = 0b0001,
			DIRTY_UNIFORMS = 0b0010,
			DIRTY_DEPTH    = 0b0100,
		};
		// clang-format on
		BitField<DirtyFlag> dirty = DIRTY_NONE;

		Vector<MDUniformSet *> uniform_sets;

		void mark_dirty() {
			if (pipeline)
				dirty.set_flag(DIRTY_PIPELINE);
		}
		_FORCE_INLINE_ void reset() {
			pass = nil;
			frameBuffer = nil;
			pipeline = nil;
			current_subpass = UINT32_MAX;
			render_area = {};
			is_rendering_entire_area = false;
			encoder = nil;
			index_buffer = nil;
			index_type = MTLIndexTypeUInt16;
			dirty = DIRTY_NONE;
			uniform_sets.clear();
		}
	} render;

	struct {
		MDComputePipeline *pipeline = nullptr;
		id<MTLComputeCommandEncoder> encoder = nil;
		_FORCE_INLINE_ void reset() {
			pipeline = nil;
			encoder = nil;
		}
	} compute;

	struct {
		id<MTLBlitCommandEncoder> encoder = nil;
		_FORCE_INLINE_ void reset() {
			encoder = nil;
		}
	} blit;

	_FORCE_INLINE_ id<MTLCommandBuffer> get_command_buffer() const {
		return commandBuffer;
	}

	id<MTLCommandEncoder> get_encoder() const {
		switch (type) {
			case MDCommandBufferStateType::Render:
				return render.encoder;
			case MDCommandBufferStateType::Compute:
				return compute.encoder;
			case MDCommandBufferStateType::Blit:
				return blit.encoder;
			default:
				return nil;
		}
	}

	void begin();
	void commit();
	void end();

	id<MTLBlitCommandEncoder> blit_command_encoder();
	void encodeRenderCommandEncoderWithDescriptor(MTLRenderPassDescriptor *desc, NSString *label);

	void bind_pipeline(RDD::PipelineID pipeline);

#pragma mark - Render Commands

	void render_bind_uniform_set(RDD::UniformSetID p_uniform_set, RDD::ShaderID p_shader, uint32_t p_set_index);
	void render_clear_attachments(VectorView<RDD::AttachmentClear> p_attachment_clears, VectorView<RDD::AttachmentClearRect> p_rects);

	void render_begin_pass(RDD::RenderPassID p_render_pass,
			RDD::FramebufferID p_frameBuffer,
			RDD::CommandBufferType p_cmd_buffer_type,
			const Rect2i &p_rect,
			VectorView<RDD::RenderPassClearValue> p_clear_values);
	void render_draw(uint32_t p_vertex_count,
			uint32_t p_instance_count,
			uint32_t p_base_vertex,
			uint32_t p_first_instance);
	void render_bind_index_buffer(RDD::BufferID p_buffer, RDD::IndexBufferFormat p_format, uint64_t p_offset);
	void render_draw_indexed(uint32_t p_index_count,
			uint32_t p_instance_count,
			uint32_t p_first_index,
			int32_t p_vertex_offset,
			uint32_t p_first_instance);
	void render_end_pass();

#pragma mark - Compute Commands

	void compute_bind_uniform_set(RDD::UniformSetID p_uniform_set, RDD::ShaderID p_shader, uint32_t p_set_index);

	MDCommandBuffer(id<MTLCommandQueue> p_queue, MetalContext *p_context) :
			queue(p_queue), context(p_context) {
		type = MDCommandBufferStateType::None;
	}

	MDCommandBuffer() = default;
};

struct BindingInfo {
	MTLDataType dataType = MTLDataTypeNone;
	uint32_t index = 0;
	MTLBindingAccess access = MTLBindingAccessReadOnly;
	MTLResourceUsage usage = 0;
	MTLTextureType textureType = MTLTextureType2D;
	spv::ImageFormat imageFormat = spv::ImageFormatUnknown;
	uint32_t arrayLength = 0;
	bool isMultisampled = false;

	[[nodiscard]] inline auto new_argument_descriptor() const -> MTLArgumentDescriptor * {
		MTLArgumentDescriptor *desc = MTLArgumentDescriptor.argumentDescriptor;
		desc.dataType = dataType;
		desc.index = index;
		desc.access = access;
		desc.textureType = textureType;
		desc.arrayLength = arrayLength;
		return desc;
	}
};

using RDC = RenderingDeviceCommons;

struct UniformInfo {
	uint32 binding;
	ShaderStageUsage active_stages;
	HashMap<RDC::ShaderStage, BindingInfo> bindings;
	HashMap<RDC::ShaderStage, BindingInfo> bindings_secondary;
};

struct UniformSet {
	Vector<UniformInfo> uniforms;
	uint32_t buffer_size;
	HashMap<RDC::ShaderStage, uint32_t> offsets;
	HashMap<RDC::ShaderStage, id<MTLArgumentEncoder>> encoders;
};

class MDShader {
protected:
	String name;

public:
	Vector<UniformSet> sets;

	virtual void encode_push_constant_data(VectorView<uint32_t> data, MDCommandBuffer *cb) = 0;

	MDShader(String p_name, Vector<UniformSet> p_sets) :
			name(p_name), sets(p_sets) {}
	virtual ~MDShader() = default;
};

class MDComputeShader final : public MDShader {
public:
	struct {
		NSUInteger binding = -1;
		uint32_t size = 0;
	} push_constants;
	MTLSize local = { 0 };

	id<MTLLibrary> kernel;
#if DEV_ENABLED
	NSString *kernel_source = nil;
#endif

	void encode_push_constant_data(VectorView<uint32_t> data, MDCommandBuffer *cb) final;

	MDComputeShader(String p_name, Vector<UniformSet> p_sets, id<MTLLibrary> p_kernel);
	~MDComputeShader() override = default;
};

class MDRenderShader final : public MDShader {
public:
	struct {
		struct {
			int32_t binding = -1;
			uint32_t size = 0;
		} vert;
		struct {
			int32_t binding = -1;
			uint32_t size = 0;
		} frag;
	} push_constants;

	id<MTLLibrary> vert;
	id<MTLLibrary> frag;
#if DEV_ENABLED
	NSString *vert_source = nil;
	NSString *frag_source = nil;
#endif

	void encode_push_constant_data(VectorView<uint32_t> data, MDCommandBuffer *cb) final;

	MDRenderShader(String p_name, Vector<UniformSet> p_sets, id<MTLLibrary> p_vert, id<MTLLibrary> p_frag);
	~MDRenderShader() override = default;
};

enum StageResourceUsage : uint32_t {
	VertexRead = (MTLResourceUsageRead << RDD::SHADER_STAGE_VERTEX * 2),
	VertexWrite = (MTLResourceUsageWrite << RDD::SHADER_STAGE_VERTEX * 2),
	FragmentRead = (MTLResourceUsageRead << RDD::SHADER_STAGE_FRAGMENT * 2),
	FragmentWrite = (MTLResourceUsageWrite << RDD::SHADER_STAGE_FRAGMENT * 2),
	TesselationControlRead = (MTLResourceUsageRead << RDD::SHADER_STAGE_TESSELATION_CONTROL * 2),
	TesselationControlWrite = (MTLResourceUsageWrite << RDD::SHADER_STAGE_TESSELATION_CONTROL * 2),
	TesselationEvaluationRead = (MTLResourceUsageRead << RDD::SHADER_STAGE_TESSELATION_EVALUATION * 2),
	TesselationEvaluationWrite = (MTLResourceUsageWrite << RDD::SHADER_STAGE_TESSELATION_EVALUATION * 2),
	ComputeRead = (MTLResourceUsageRead << RDD::SHADER_STAGE_COMPUTE * 2),
	ComputeWrite = (MTLResourceUsageWrite << RDD::SHADER_STAGE_COMPUTE * 2),
};

_FORCE_INLINE_ StageResourceUsage &operator|=(StageResourceUsage &a, uint32_t b) {
	a = StageResourceUsage(uint32_t(a) | b);
	return a;
}

_FORCE_INLINE_ StageResourceUsage stage_resource_usage(RDC::ShaderStage p_stage, MTLResourceUsage p_usage) {
	return StageResourceUsage(p_usage << (p_stage * 2));
}

_FORCE_INLINE_ MTLResourceUsage resource_usage_for_stage(StageResourceUsage p_usage, RDC::ShaderStage p_stage) {
	return MTLResourceUsage((p_usage >> (p_stage * 2)) & 0b11);
}

template <>
struct HashMapComparatorDefault<RDD::ShaderID> {
	static bool compare(const RDD::ShaderID &p_lhs, const RDD::ShaderID &p_rhs) {
		return p_lhs.id == p_rhs.id;
	}
};

struct BoundUniformSet {
	id<MTLBuffer> buffer;
	HashMap<id<MTLResource>, StageResourceUsage> bound_resources;
};

class MDUniformSet {
public:
	NSUInteger index;
	Vector<RDD::BoundUniform> p_uniforms;
	HashMap<MDShader *, BoundUniformSet> bound_uniforms;

	BoundUniformSet &boundUniformSetForShader(MDShader *p_shader, id<MTLDevice> device);
};
enum class MDAttachmentType : uint8_t {
	None = 0,
	Color = 1 << 0,
	Depth = 1 << 1,
	Stencil = 1 << 2,
};

_FORCE_INLINE_ MDAttachmentType &operator|=(MDAttachmentType &a, MDAttachmentType b) {
	a = MDAttachmentType(uint8_t(a) | uint8_t(b));
	return a;
}

_FORCE_INLINE_ bool operator&(MDAttachmentType a, MDAttachmentType b) {
	return uint8_t(a) & uint8_t(b);
}

struct MDAttachment {
	MTLPixelFormat format = MTLPixelFormatInvalid;
	MDAttachmentType type = MDAttachmentType::None;
	MTLLoadAction loadAction = MTLLoadActionDontCare;
	MTLStoreAction storeAction = MTLStoreActionDontCare;
	uint32_t samples = 1;
};

class MDRenderPass {
	NSInteger _depthIndex;
	NSInteger _stencilIndex;

public:
	TightLocalVector<MDAttachment> attachments;
	TightLocalVector<RDD::Subpass> subpasses;

	[[nodiscard]] MDAttachment const *depth() const {
		return _depthIndex == NSNotFound ? nullptr : &attachments[_depthIndex];
	}

	[[nodiscard]] MDAttachment const *stencil() const {
		return _stencilIndex == NSNotFound ? nullptr : &attachments[_stencilIndex];
	}

	uint32_t get_sample_count() const {
		return attachments.is_empty() ? 1 : attachments[0].samples;
	};

	MDRenderPass(TightLocalVector<MDAttachment> &&p_attachments, TightLocalVector<RDD::Subpass> &&p_subpasses, NSInteger p_depthIndex, NSInteger p_stencilIndex) :
			attachments(p_attachments), subpasses(p_subpasses), _depthIndex(p_depthIndex), _stencilIndex(p_stencilIndex) {}

	explicit MDRenderPass(TightLocalVector<MDAttachment> &&p_attachments, TightLocalVector<RDD::Subpass> &&p_subpasses) :
			MDRenderPass(std::move(p_attachments), std::move(p_subpasses), NSNotFound, NSNotFound) {}
};

class MDPipeline {
public:
	MDPipelineType type;

	explicit MDPipeline(MDPipelineType p_type) :
			type(p_type) {}
	virtual ~MDPipeline() = default;
};

class MDRenderPipeline final : public MDPipeline {
public:
	id<MTLRenderPipelineState> state = nil;
	id<MTLDepthStencilState> depth_stencil = nil;
	uint32_t push_constant_size = 0;
	uint32_t push_constant_stages_mask = 0;

	struct {
		MTLCullMode cull_mode = MTLCullModeNone;
		MTLTriangleFillMode fill_mode = MTLTriangleFillModeFill;
		MTLDepthClipMode clip_mode = MTLDepthClipModeClip;
		MTLWinding winding = MTLWindingClockwise;
		MTLPrimitiveType render_primitive = MTLPrimitiveTypePoint;

		struct {
			bool enabled = false;
		} depth_test;

		struct {
			bool enabled = false;
			float depth_bias = 0.0;
			float slope_scale = 0.0;
			float clamp = 0.0;
			_FORCE_INLINE_ void apply(id<MTLRenderCommandEncoder> enc) const {
				if (!enabled)
					return;
				[enc setDepthBias:depth_bias slopeScale:slope_scale clamp:clamp];
			};
		} depth_bias;

		struct {
			bool enabled = false;
			uint32_t front_reference = 0;
			uint32_t back_reference = 0;
			_FORCE_INLINE_ void apply(id<MTLRenderCommandEncoder> enc) const {
				if (!enabled)
					return;
				[enc setStencilFrontReferenceValue:front_reference backReferenceValue:back_reference];
			};
		} stencil;

		struct {
			bool enabled = false;
			float r = 0.0;
			float g = 0.0;
			float b = 0.0;
			float a = 0.0;

			_FORCE_INLINE_ void apply(id<MTLRenderCommandEncoder> enc) const {
				if (!enabled)
					return;
				[enc setBlendColorRed:r green:g blue:b alpha:a];
			};
		} blend;

		_FORCE_INLINE_ void apply(id<MTLRenderCommandEncoder> enc) const {
			[enc setCullMode:cull_mode];
			[enc setTriangleFillMode:fill_mode];
			[enc setDepthClipMode:clip_mode];
			[enc setFrontFacingWinding:winding];
			depth_bias.apply(enc);
			stencil.apply(enc);
			blend.apply(enc);
		};

	} raster_state;
#if DEV_ENABLED
	MDRenderShader *shader = nil;
#endif

	MDRenderPipeline() :
			MDPipeline(MDPipelineType::Render) {}
	~MDRenderPipeline() final = default;
};

class MDComputePipeline final : public MDPipeline {
public:
	id<MTLComputePipelineState> state = nil;
	struct {
		MTLSize local = { 0 };
	} compute_state;
#if DEV_ENABLED
	MDComputeShader *shader = nil;
#endif

	explicit MDComputePipeline(id<MTLComputePipelineState> p_state) :
			MDPipeline(MDPipelineType::Compute), state(p_state) {}
	~MDComputePipeline() final = default;
};

class MDFrameBuffer {
public:
	Vector<MTL::Texture> textures;
	Size2i size;
	MDFrameBuffer(Vector<MTL::Texture> p_textures, Size2i p_size) :
			textures(p_textures), size(p_size) {}

	MTLRenderPassDescriptor *newRenderPassDescriptorWithRenderPass(MDRenderPass *pass, VectorView<RDD::RenderPassClearValue> colors) const;
	virtual ~MDFrameBuffer() = default;
};

class MDScreenFrameBuffer final : public MDFrameBuffer {
public:
	id<CAMetalDrawable> drawable;
	explicit MDScreenFrameBuffer(id<CAMetalDrawable> p_drawable, Size2i p_size) :
			MDFrameBuffer(Vector<MTL::Texture>({ p_drawable.texture }), p_size), drawable(p_drawable) {}
	~MDScreenFrameBuffer() final = default;
};

class MDQueryPool {
	// GPU counters
	NSUInteger sampleCount = 0;
	id<MTLCounterSet> counterSet = nil;
	id<MTLCounterSampleBuffer> _counterSampleBuffer = nil;
	// sampling
	double cpuStart = 0.0;
	double gpuStart = 0.0;
	double cpuTimeSpan = 0.0;
	double gpuTimeSpan = 0.0;

	// buffer
	LocalVector<double> results;

	void resolveSampleBuffer();

public:
	[[nodiscard]] id<MTLCounterSampleBuffer> get_counter_sample_buffer() const { return _counterSampleBuffer; }

	void reset_with_command_buffer(RDD::CommandBufferID p_cmd_buffer);
	void get_results(uint64_t *p_results, NSUInteger count);
	void write_command_buffer(RDD::CommandBufferID p_cmd_buffer, NSUInteger index);

	static std::shared_ptr<MDQueryPool> new_query_pool(id<MTLDevice> device, NSError **error);

	~MDQueryPool() = default;

private:
	MDQueryPool() = default;
};

namespace rid2 {
template <typename U, typename T>
U to_id(std::shared_ptr<T> p_obj) {
	return U(new std::shared_ptr<T>(p_obj));
}

template <typename T>
void release(RDD::ID p_id) {
	auto *sp = (std::shared_ptr<T> *)p_id.id;
	delete sp;
}

template <typename T>
std::shared_ptr<T> get(RDD::ID p_id) {
	auto *sp = (std::shared_ptr<T> *)p_id.id;
	return *sp;
}
} //namespace rid2

/// These functions are used to convert between Objective-C objects and
/// the RIDs used by Godot, respecting automatic reference counting.
namespace rid {
/// owned converts an Objective C object to a pointer, and incrementing the
/// reference count.
_ALWAYS_INLINE_
void *owned(id p_id) {
	return (__bridge_retained void *)p_id;
}

/// unowned converts an Objective C object to a pointer, without incrementing
/// the reference count.
_ALWAYS_INLINE_
void *unowned(id p_id) {
	return (__bridge void *)p_id;
}

#define MAKE_ID(FROM, TO)                                            \
	_ALWAYS_INLINE_ TO make(FROM p_obj) { return TO(owned(p_obj)); } \
	_ALWAYS_INLINE_ TO make_unowned(FROM p_obj) { return TO(unowned(p_obj)); }

MAKE_ID(id<MTLTexture>, RDD::TextureID)
MAKE_ID(id<MTLBuffer>, RDD::BufferID)
MAKE_ID(id<MTLSamplerState>, RDD::SamplerID)
MAKE_ID(MTLVertexDescriptor *, RDD::VertexFormatID)
MAKE_ID(id<MTLCommandQueue>, RDD::CommandPoolID)

/// get converts a pointer to an Objective C object without changing the reference count.
_ALWAYS_INLINE_
auto get(RDD::ID p_id) {
	return (p_id.id) ? (__bridge ::id)(void *)p_id.id : nil;
}

/// release converts a pointer to an Objective C object, and decrements the reference count.
_ALWAYS_INLINE_
auto release(RDD::ID p_id) {
	return (__bridge_transfer ::id)(void *)p_id.id;
}

} //namespace rid

#endif //GODOT_METAL_OBJECTS_H
