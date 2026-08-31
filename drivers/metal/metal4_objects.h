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

#include "metal_objects_shared.h"
#include "rendering_device_driver_metal.h"

#include "servers/rendering/rendering_device_driver.h"

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <zlib.h>

#include <initializer_list>
#include <optional>

namespace MTL4 {

class RenderingDeviceDriverMetal;
class MD4CommandPool;
class QueryPool;

// These types are defined in the global namespace (metal_objects_shared.h / rendering_device_driver_metal.h)
using ::MDAttachment;
using ::MDAttachmentType;
using ::MDCommandBufferBase;
using ::MDCommandBufferStateType;
using ::MDFrameBuffer;
using ::MDRenderPass;
using ::MDRingBuffer;
using ::MDShader;
using ::MDSubpass;
using ::MDUniformSet;
using ::MetalBufferDynamicInfo;
using ::RenderStateBase;

using RDM = RenderingDeviceDriverMetal;

class API_AVAILABLE(macos(26.0), ios(26.0), tvos(26.0), visionos(26.0)) MDCommandBuffer : public MDCommandBufferBase {
	friend class MDUniformSet;

private:
#pragma mark - Common State

	/// Default size for the per-frame scratch buffers is 2MiB.
	static constexpr uint32_t DEFAULT_SCRATCH_SIZE = 1024 * 1024 * 2;

	// Level-fence hooks. Called once per encoder: _fence_wait right after
	// creation, _fence_update immediately before endEncoding.
	void _fence_wait(MTL4::RenderCommandEncoder *p_enc);
	void _fence_wait(MTL4::ComputeCommandEncoder *p_enc);
	void _fence_update(MTL4::RenderCommandEncoder *p_enc);
	void _fence_update(MTL4::ComputeCommandEncoder *p_enc);

	void reset();

	NS::SharedPtr<MTL4::CommandAllocator> allocator;
	NS::SharedPtr<MTL4::CommandBuffer> command_buffer;
	bool state_begin = false;

	MDRingBuffer _scratch;
	// Used by render_clear_attachments
	NS::SharedPtr<MTL4::ArgumentTable> _args_clear;

	void _end_compute();
	void _end_inline_render();
	void _pop_active_encoder_labels();
	void _set_inline_render_encoder(MTL4::RenderCommandEncoder *p_encoder);
	// Metal 4 folds blit commands into the compute encoder, so one encoder serves
	// every compute list and transfer in a graph level. It opens on first use and
	// closes at the level boundary (command_group_end) or when a render pass starts.
	MTL4::ComputeCommandEncoder *_ensure_compute_encoder();

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
	void _bind_uniforms_direct(MDUniformSet *p_set, MDShader *p_shader, MTL::ResidencySet *p_rs, MTL4::ArgumentTable *p_args, uint32_t p_set_index, uint32_t p_dynamic_offsets);

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
	void end_render_encoding() override {
		_fence_update(render.encoder.get());
		render.end_encoding();
	}

public:
	struct RenderState : public RenderStateBase {
		MDRenderPass *pass = nullptr;
		MDFrameBuffer *frameBuffer = nullptr;
		MDRenderPipeline *pipeline = nullptr;
		LocalVector<RDD::RenderPassClearValue> clear_values;
		MDSubpass *current_subpass = nullptr;
		Rect2i render_area = {};
		bool is_rendering_entire_area = false;
		NS::SharedPtr<MTL4::RenderPassDescriptor> desc;
		NS::SharedPtr<MTL4::RenderCommandEncoder> encoder;
		NS::SharedPtr<MTL4::ArgumentTable> args;
		MTL::Buffer *index_buffer = nullptr; // Buffer is owned by RDD.
		MTL::IndexType index_type = MTL::IndexTypeUInt16;
		_FORCE_INLINE_ size_t index_type_size() const { return index_type == MTL::IndexTypeUInt16 ? sizeof(uint16_t) : sizeof(uint32_t); }
		uint32_t index_offset = 0;
		LocalVector<MTL::Buffer *> vertex_buffers;
		LocalVector<NS::UInteger> vertex_offsets;
		NS::SharedPtr<MTL::ResidencySet> residency_set;

		LocalVector<MDUniformSet *> uniform_sets;
		uint32_t dynamic_offsets = 0;
		// Bit mask of the uniform sets that are dirty, to prevent redundant binding.
		uint64_t uniform_set_mask = 0;

		// Mirror of the raster state currently programmed into the active encoder. A freshly
		// created render command encoder starts at Metal's defaults (fill Fill, clip Clip,
		// winding Clockwise, cull None, bias 0, stencil ref 0, blend 0), which are exactly the
		// defaults of RasterState, so no "valid" flag is needed. Reset it whenever a new
		// encoder is created, and update it anywhere raster state is written directly.
		MDRenderPipeline::RasterState encoder_raster;

		_FORCE_INLINE_ void reset();
		void end_encoding();

		_ALWAYS_INLINE_ const MDSubpass &get_subpass() const {
			DEV_ASSERT(current_subpass != nullptr);
			return *current_subpass;
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

		MTL::ScissorRect clip_to_render_area(MTL::ScissorRect p_rect) const {
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
		NS::SharedPtr<MTL4::ComputeCommandEncoder> encoder;
		NS::SharedPtr<MTL4::ArgumentTable> args;
		NS::SharedPtr<MTL::ResidencySet> residency_set;
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

		// Clears the per-list bindings but keeps the encoder open.
		_FORCE_INLINE_ void reset_bindings();
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

	struct {
		NS::SharedPtr<MTL4::RenderCommandEncoder> encoder;

		_FORCE_INLINE_ void reset() {
			encoder.reset();
		}
	} inline_render;

	_FORCE_INLINE_ MTL4::CommandBuffer *get_command_buffer() const {
		return command_buffer.get();
	}

	void _begin() override;
	void _commit() override;
	void _end() override;

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

	void compute_begin_pass() override;
	void compute_end_pass() override;
	void compute_bind_uniform_sets(VectorView<RDD::UniformSetID> p_uniform_sets, RDD::ShaderID p_shader, uint32_t p_first_set_index, uint32_t p_set_count, uint32_t p_dynamic_offsets) override;
	void compute_dispatch(uint32_t p_x_groups, uint32_t p_y_groups, uint32_t p_z_groups) override;
	void compute_dispatch_indirect(RDD::BufferID p_indirect_buffer, uint64_t p_offset) override;

#pragma mark - Transfer

private:
	MTL4::RenderCommandEncoder *get_new_render_encoder_with_descriptor(MTL4::RenderPassDescriptor *p_desc);

public:
	void resolve_texture(RDD::TextureID p_src_texture, RDD::TextureLayout p_src_texture_layout, uint32_t p_src_layer, uint32_t p_src_mipmap, RDD::TextureID p_dst_texture, RDD::TextureLayout p_dst_texture_layout, uint32_t p_dst_layer, uint32_t p_dst_mipmap) override;
	void clear_color_texture(RDD::TextureID p_texture, RDD::TextureLayout p_texture_layout, const Color &p_color, const RDD::TextureSubresourceRange &p_subresources) override;
	void clear_depth_stencil_texture(RDD::TextureID p_texture, RDD::TextureLayout p_texture_layout, float p_depth, uint8_t p_stencil, const RDD::TextureSubresourceRange &p_subresources) override;
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
			VectorView<RDD::TextureBarrier> p_texture_barriers,
			VectorView<RDD::AccelerationStructureBarrier> p_acceleration_structure_barriers) override;

#pragma mark - Timestamp

	void timestamp_write(QueryPool *p_pool, uint32_t p_index);

#pragma mark - Debugging

	void begin_label(const char *p_label_name, const Color &p_color) override;
	void end_label() override;

	MDCommandBuffer(MTL4::CommandAllocator *p_allocator, RenderingDeviceDriverMetal *p_device_driver);

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

// C++ helper to get mipmap level size from texture
_FORCE_INLINE_ static MTL::Size mipmapLevelSizeFromTexture(MTL::Texture *p_tex, NS::UInteger p_level) {
	MTL::Size lvlSize;
	lvlSize.width = MAX(p_tex->width() >> p_level, 1UL);
	lvlSize.height = MAX(p_tex->height() >> p_level, 1UL);
	lvlSize.depth = MAX(p_tex->depth() >> p_level, 1UL);
	return lvlSize;
}

class API_AVAILABLE(macos(26.0), ios(26.0), tvos(26.0), visionos(26.0)) QueryPool {
	NS::SharedPtr<MTL4::CounterHeap> counter_heap;
	uint64_t timestamp_frequency = 0;
	double timestamp_to_nano = 0.0;
	uint32_t count = 0;

public:
	QueryPool(NS::SharedPtr<MTL4::CounterHeap> p_heap, uint32_t p_count, uint64_t p_frequency);

	MTL4::CounterHeap *get_counter_heap() const { return counter_heap.get(); }
	uint64_t get_timestamp_frequency() const { return timestamp_frequency; }
	uint32_t get_count() const { return count; }

	void invalidate(uint32_t p_count);
	void get_results(uint32_t p_count, uint64_t *r_results);
	uint64_t result_to_time(uint64_t p_result) const { return uint64_t((double)p_result * timestamp_to_nano); }
};

} // namespace MTL4

namespace rid {
#define MAKE_ID(FROM, TO) \
	API_AVAILABLE(macos(26), ios(26), tvos(26), visionos(26)) \
	_FORCE_INLINE_ TO make(FROM p_obj) { \
		return TO(reinterpret_cast<uint64_t>(p_obj)); \
	}

MAKE_ID(MTL4::CommandQueue *, RDD::CommandQueueID);

#undef MAKE_ID

// Note: rid::get<T> templates are defined in metal_objects_shared.h

} //namespace rid
