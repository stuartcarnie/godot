/**************************************************************************/
/*  metal4_objects.h                                                      */
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

/**************************************************************************/
/*                                                                        */
/* Portions of this code were derived from MoltenVK.                      */
/*                                                                        */
/* Copyright (c) 2015-2023 The Brenwill Workshop Ltd.                     */
/* (http://www.brenwill.com)                                              */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/**************************************************************************/

#import "metal_objects_shared.h"
#import "rendering_device_driver_metal.h"

#include "servers/rendering/rendering_device_driver.h"

#import <CommonCrypto/CommonDigest.h>
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <simd/simd.h>
#import <zlib.h>
#import <initializer_list>
#import <optional>

namespace MTL4 {

class RenderingDeviceDriverMetal;
class MD4CommandPool;

// These types are defined in the global namespace (metal_objects_shared.h / rendering_device_driver_metal.h)
using ::MDAttachment;
using ::MDAttachmentType;
using ::MDCommandBufferBase;
using ::MDCommandBufferStateType;
using ::MDFrameBuffer;
using ::MDRenderPass;
using ::MDRingBuffer;
using ::MDSubpass;
using ::MetalBufferDynamicInfo;
using ::RenderStateBase;

using RDM = RenderingDeviceDriverMetal;

class API_AVAILABLE(macos(26.0), ios(26.0), tvos(26.0), visionos(26.0)) MDCommandBuffer : public MDCommandBufferBase {
	friend class ::MDUniformSet;

private:
#pragma mark - Common State

	/// Default size for the per-frame scratch buffers is 2MiB.
	static constexpr uint32_t DEFAULT_SCRATCH_SIZE = 1024 * 1024 * 2;

	enum {
		STAGE_RENDER,
		STAGE_COMPUTE,
		STAGE_MAX,
	};
	MTLStages pending_after_stages[STAGE_MAX] = { 0, 0 };
	MTLStages pending_before_queue_stages[STAGE_MAX] = { 0, 0 };
	void _encode_barrier(id<MTL4CommandEncoder> p_enc);

	void reset();

	id<MTL4CommandAllocator> allocator = nil;
	id<MTL4CommandBuffer> command_buffer = nil;
	bool state_begin = false;

	struct PendingBarrier {
		MTLStages src_stages;
		MTLStages dst_stages;
		MTL4VisibilityOptions visibility;
	};

	struct {
		id<MTLResidencySet> rs = nil;
	} _frame_state;

	MDRingBuffer _scratch;
	// Used by render_clear_attachments
	id<MTL4ArgumentTable> _args_clear;

	void _end_compute_dispatch();
	id<MTL4ComputeCommandEncoder> _ensure_blit_encoder();

	enum class CopySource {
		Buffer,
		Texture,
	};
	void _copy_texture_buffer(CopySource p_source,
			RDD::TextureID p_texture,
			RDD::BufferID p_buffer,
			VectorView<RDD::BufferTextureCopyRegion> p_regions);

#pragma mark - Render

	void _render_set_dirty_state();
	void _render_bind_uniform_sets();
	void _bind_uniforms_argument_buffers(MDUniformSet *p_set, MDShader *p_shader, uint32_t p_set_index, uint32_t p_dynamic_offsets);
	void _bind_uniforms_direct(MDUniformSet *p_set, MDShader *p_shader, id<MTLResidencySet> p_rs, id<MTL4ArgumentTable> p_args, uint32_t p_set_index, uint32_t p_dynamic_offsets);

#pragma mark - Compute

	void _compute_set_dirty_state();
	void _compute_bind_uniform_sets();
	void _bind_uniforms_argument_buffers_compute(MDUniformSet *p_set, MDShader *p_shader, uint32_t p_set_index, uint32_t p_dynamic_offsets);

protected:
	void mark_push_constants_dirty() override;
	RenderStateBase &get_render_state_base() override { return render; }
	uint32_t get_current_view_count() const override { return render.get_subpass().view_count; }
	MDRenderPass *get_render_pass() const override { return render.pass; }
	MDFrameBuffer *get_frame_buffer() const override { return render.frameBuffer; }
	const MDSubpass &get_current_subpass() const override { return render.get_subpass(); }
	LocalVector<RDD::RenderPassClearValue> &get_clear_values() override { return render.clear_values; }
	const Rect2i &get_render_area() const override { return render.render_area; }
	void end_render_encoding() override { render.end_encoding(); }

public:
	struct RenderState : public RenderStateBase {
		MDRenderPass *pass = nullptr;
		MDFrameBuffer *frameBuffer = nullptr;
		MDRenderPipeline *pipeline = nullptr;
		LocalVector<RDD::RenderPassClearValue> clear_values;
		uint32_t current_subpass = UINT32_MAX;
		Rect2i render_area = {};
		bool is_rendering_entire_area = false;
		MTL4RenderPassDescriptor *desc = nil;
		id<MTL4RenderCommandEncoder> encoder = nil;
		id<MTL4ArgumentTable> args = nil;
		id<MTLBuffer> __unsafe_unretained index_buffer = nil; // Buffer is owned by RDD.
		MTLIndexType index_type = MTLIndexTypeUInt16;
		_FORCE_INLINE_ size_t index_type_size() const { return index_type == MTLIndexTypeUInt16 ? sizeof(uint16_t) : sizeof(uint32_t); }
		uint32_t index_offset = 0;
		LocalVector<id<MTLBuffer> __unsafe_unretained> vertex_buffers;
		LocalVector<NSUInteger> vertex_offsets;
		id<MTLResidencySet> residency_set;

		LocalVector<MDUniformSet *> uniform_sets;
		uint32_t dynamic_offsets = 0;
		// Bit mask of the uniform sets that are dirty, to prevent redundant binding.
		uint64_t uniform_set_mask = 0;

		_FORCE_INLINE_ void reset();
		void end_encoding();

		_ALWAYS_INLINE_ const MDSubpass &get_subpass() const {
			DEV_ASSERT(pass != nullptr);
			return pass->subpasses[current_subpass];
		}

		_FORCE_INLINE_ void mark_viewport_dirty() {
			if (viewports.is_empty()) {
				return;
			}
			dirty.set_flag(DirtyFlag::DIRTY_VIEWPORT);
		}

		_FORCE_INLINE_ void mark_scissors_dirty() {
			if (scissors.is_empty()) {
				return;
			}
			dirty.set_flag(DirtyFlag::DIRTY_SCISSOR);
		}

		_FORCE_INLINE_ void mark_vertex_dirty() {
			if (vertex_buffers.is_empty()) {
				return;
			}
			dirty.set_flag(DirtyFlag::DIRTY_VERTEX);
		}

		_FORCE_INLINE_ void mark_uniforms_dirty(std::initializer_list<uint32_t> l) {
			if (uniform_sets.is_empty()) {
				return;
			}
			for (uint32_t i : l) {
				if (i < uniform_sets.size() && uniform_sets[i] != nullptr) {
					uniform_set_mask |= 1 << i;
				}
			}
			dirty.set_flag(DirtyFlag::DIRTY_UNIFORMS);
		}

		_FORCE_INLINE_ void mark_uniforms_dirty(void) {
			if (uniform_sets.is_empty()) {
				return;
			}
			for (uint32_t i = 0; i < uniform_sets.size(); i++) {
				if (uniform_sets[i] != nullptr) {
					uniform_set_mask |= 1 << i;
				}
			}
			dirty.set_flag(DirtyFlag::DIRTY_UNIFORMS);
		}

		_FORCE_INLINE_ void mark_blend_dirty() {
			if (!blend_constants.has_value()) {
				return;
			}
			dirty.set_flag(DirtyFlag::DIRTY_BLEND);
		}

		MTLScissorRect clip_to_render_area(MTLScissorRect p_rect) const {
			uint32_t raLeft = render_area.position.x;
			uint32_t raRight = raLeft + render_area.size.width;
			uint32_t raBottom = render_area.position.y;
			uint32_t raTop = raBottom + render_area.size.height;

			p_rect.x = CLAMP(p_rect.x, raLeft, MAX(raRight - 1, raLeft));
			p_rect.y = CLAMP(p_rect.y, raBottom, MAX(raTop - 1, raBottom));
			p_rect.width = MIN(p_rect.width, raRight - p_rect.x);
			p_rect.height = MIN(p_rect.height, raTop - p_rect.y);

			return p_rect;
		}

		Rect2i clip_to_render_area(Rect2i p_rect) const {
			int32_t raLeft = render_area.position.x;
			int32_t raRight = raLeft + render_area.size.width;
			int32_t raBottom = render_area.position.y;
			int32_t raTop = raBottom + render_area.size.height;

			p_rect.position.x = CLAMP(p_rect.position.x, raLeft, MAX(raRight - 1, raLeft));
			p_rect.position.y = CLAMP(p_rect.position.y, raBottom, MAX(raTop - 1, raBottom));
			p_rect.size.width = MIN(p_rect.size.width, raRight - p_rect.position.x);
			p_rect.size.height = MIN(p_rect.size.height, raTop - p_rect.position.y);

			return p_rect;
		}

	} render;

	// State specific for a compute pass.
	struct ComputeState {
		MDComputePipeline *pipeline = nullptr;
		id<MTL4ComputeCommandEncoder> encoder = nil;
		id<MTL4ArgumentTable> args = nil;
		id<MTLResidencySet> residency_set;
		// clang-format off
		enum DirtyFlag: uint16_t {
			DIRTY_NONE     = 0,
			DIRTY_PIPELINE = 1 << 0, //! pipeline state
			DIRTY_UNIFORMS = 1 << 1, //! uniform sets
			DIRTY_PUSH     = 1 << 2, //! push constants
			DIRTY_ALL      = (1 << 3) - 1,
		};
		// clang-format on
		BitField<DirtyFlag> dirty = DIRTY_NONE;

		LocalVector<MDUniformSet *> uniform_sets;
		uint32_t dynamic_offsets = 0;
		// Bit mask of the uniform sets that are dirty, to prevent redundant binding.
		uint64_t uniform_set_mask = 0;

		_FORCE_INLINE_ void reset();
		void end_encoding();

		_FORCE_INLINE_ void mark_uniforms_dirty(void) {
			if (uniform_sets.is_empty()) {
				return;
			}
			for (uint32_t i = 0; i < uniform_sets.size(); i++) {
				if (uniform_sets[i] != nullptr) {
					uniform_set_mask |= 1 << i;
				}
			}
			dirty.set_flag(DirtyFlag::DIRTY_UNIFORMS);
		}
	} compute;

	_FORCE_INLINE_ id<MTL4CommandBuffer> get_command_buffer() const {
		return command_buffer;
	}

	void begin() override;
	void commit() override;
	void end() override;

	void bind_pipeline(RDD::PipelineID p_pipeline) override;

#pragma mark - Render Commands

	void render_bind_uniform_sets(VectorView<RDD::UniformSetID> p_uniform_sets, RDD::ShaderID p_shader, uint32_t p_first_set_index, uint32_t p_set_count, uint32_t p_dynamic_offsets) override;
	void render_clear_attachments(VectorView<RDD::AttachmentClear> p_attachment_clears, VectorView<Rect2i> p_rects) override;
	void render_begin_pass(RDD::RenderPassID p_render_pass,
			RDD::FramebufferID p_frameBuffer,
			RDD::CommandBufferType p_cmd_buffer_type,
			const Rect2i &p_rect,
			VectorView<RDD::RenderPassClearValue> p_clear_values) override;
	void render_next_subpass() override;
	void render_draw(uint32_t p_vertex_count,
			uint32_t p_instance_count,
			uint32_t p_base_vertex,
			uint32_t p_first_instance) override;
	void render_bind_vertex_buffers(uint32_t p_binding_count, const RDD::BufferID *p_buffers, const uint64_t *p_offsets, uint64_t p_dynamic_offsets) override;
	void render_bind_index_buffer(RDD::BufferID p_buffer, RDD::IndexBufferFormat p_format, uint64_t p_offset) override;

	void render_draw_indexed(uint32_t p_index_count,
			uint32_t p_instance_count,
			uint32_t p_first_index,
			int32_t p_vertex_offset,
			uint32_t p_first_instance) override;

	void render_draw_indexed_indirect(RDD::BufferID p_indirect_buffer, uint64_t p_offset, uint32_t p_draw_count, uint32_t p_stride) override;
	void render_draw_indexed_indirect_count(RDD::BufferID p_indirect_buffer, uint64_t p_offset, RDD::BufferID p_count_buffer, uint64_t p_count_buffer_offset, uint32_t p_max_draw_count, uint32_t p_stride) override;
	void render_draw_indirect(RDD::BufferID p_indirect_buffer, uint64_t p_offset, uint32_t p_draw_count, uint32_t p_stride) override;
	void render_draw_indirect_count(RDD::BufferID p_indirect_buffer, uint64_t p_offset, RDD::BufferID p_count_buffer, uint64_t p_count_buffer_offset, uint32_t p_max_draw_count, uint32_t p_stride) override;

	void render_end_pass() override;

#pragma mark - Compute Commands

	void compute_bind_uniform_sets(VectorView<RDD::UniformSetID> p_uniform_sets, RDD::ShaderID p_shader, uint32_t p_first_set_index, uint32_t p_set_count, uint32_t p_dynamic_offsets) override;
	void compute_dispatch(uint32_t p_x_groups, uint32_t p_y_groups, uint32_t p_z_groups) override;
	void compute_dispatch_indirect(RDD::BufferID p_indirect_buffer, uint64_t p_offset) override;

#pragma mark - Transfer

private:
	id<MTL4RenderCommandEncoder> get_new_render_encoder_with_descriptor(MTL4RenderPassDescriptor *p_desc);

public:
	void resolve_texture(RDD::TextureID p_src_texture, RDD::TextureLayout p_src_texture_layout, uint32_t p_src_layer, uint32_t p_src_mipmap, RDD::TextureID p_dst_texture, RDD::TextureLayout p_dst_texture_layout, uint32_t p_dst_layer, uint32_t p_dst_mipmap) override;
	void clear_color_texture(RDD::TextureID p_texture, RDD::TextureLayout p_texture_layout, const Color &p_color, const RDD::TextureSubresourceRange &p_subresources) override;
	void clear_buffer(RDD::BufferID p_buffer, uint64_t p_offset, uint64_t p_size) override;
	void copy_buffer(RDD::BufferID p_src_buffer, RDD::BufferID p_dst_buffer, VectorView<RDD::BufferCopyRegion> p_regions) override;
	void copy_texture(RDD::TextureID p_src_texture, RDD::TextureID p_dst_texture, VectorView<RDD::TextureCopyRegion> p_regions) override;
	void copy_buffer_to_texture(RDD::BufferID p_src_buffer, RDD::TextureID p_dst_texture, VectorView<RDD::BufferTextureCopyRegion> p_regions) override;
	void copy_texture_to_buffer(RDD::TextureID p_src_texture, RDD::BufferID p_dst_buffer, VectorView<RDD::BufferTextureCopyRegion> p_regions) override;

#pragma mark - Synchronization

	void pipeline_barrier(BitField<RDD::PipelineStageBits> p_src_stages,
			BitField<RDD::PipelineStageBits> p_dst_stages,
			VectorView<RDD::MemoryAccessBarrier> p_memory_barriers,
			VectorView<RDD::BufferBarrier> p_buffer_barriers,
			VectorView<RDD::TextureBarrier> p_texture_barriers) override;

#pragma mark - Debugging

	void begin_label(const char *p_label_name, const Color &p_color) override;
	void end_label() override;

	MDCommandBuffer(id<MTL4CommandAllocator> p_allocator, RenderingDeviceDriverMetal *p_device_driver);

	MDCommandBuffer() = default;
	~MDCommandBuffer();
};

class API_AVAILABLE(macos(26.0), ios(26.0), tvos(26.0), visionos(26.0)) MD4CommandPool {
	RenderingDeviceDriverMetal *_driver;
	LocalVector<MDCommandBuffer *> _command_buffers;

public:
	MDCommandBuffer *new_command_buffer();

	MD4CommandPool(RenderingDeviceDriverMetal *p_driver);
	~MD4CommandPool();
};

using ::DynamicOffsetLayout;
using ::MDComputePipeline;
using ::MDComputeShader;
using ::MDLibrary;
using ::MDPipeline;
using ::MDPipelineType;
using ::MDRenderPipeline;
using ::MDRenderShader;
using ::MDShader;
using ::MDUniformSet;
using ::ShaderCacheEntry;
using ::ShaderLoadStrategy;
using ::UniformInfo;
using ::UniformSet;

} // namespace MTL4

namespace rid {
#define MAKE_ID(FROM, TO)                                     \
	API_AVAILABLE(macos(26), ios(26), tvos(26), visionos(26)) \
	_FORCE_INLINE_ TO make(FROM p_obj) {                      \
		return TO(owned(p_obj));                              \
	}

MAKE_ID(id<MTL4CommandQueue>, RDD::CommandQueueID);

#undef MAKE_ID
} //namespace rid
