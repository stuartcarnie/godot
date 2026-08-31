/**************************************************************************/
/*  metal4_objects.cpp                                                    */
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

#ifdef METAL4_ENABLED

#include "metal4_objects.h"

#include "metal_utils.h"
#include "pixel_formats.h"
#include "rendering_device_driver_metal4.h"
#include "rendering_shader_container_metal.h"

#include <algorithm>

using namespace MTL4;

GODOT_CLANG_WARNING_PUSH_AND_IGNORE("-Wunguarded-availability-new")

MDCommandBuffer::MDCommandBuffer(MTL4::CommandAllocator *p_allocator, RenderingDeviceDriverMetal *p_device_driver) :
		allocator(NS::TransferPtr(p_allocator)), command_buffer(NS::TransferPtr(p_device_driver->get_device()->newCommandBuffer())), _scratch(p_device_driver->get_allocator(), DEFAULT_SCRATCH_SIZE) {
	device_driver = p_device_driver;
	MTL::Device *device = device_driver->get_device();

	_create_level_fences(device);

	NS::SharedPtr<MTL4::ArgumentTableDescriptor> desc = NS::TransferPtr(MTL4::ArgumentTableDescriptor::alloc()->init());
	desc->setMaxBufferBindCount(31);
	desc->setMaxTextureBindCount(64);
	desc->setMaxSamplerStateBindCount(16);

	render.args = NS::TransferPtr(device->newArgumentTable(desc.get(), nullptr));
	compute.args = NS::TransferPtr(device->newArgumentTable(desc.get(), nullptr));
	_args_clear = NS::TransferPtr(device->newArgumentTable(desc.get(), nullptr));
}

MDCommandBuffer::~MDCommandBuffer() {
}

void MDCommandBuffer::begin_label(const char *p_label_name, const Color &p_color) {
	NS::SharedPtr<NS::String> s = NS::TransferPtr(NS::String::alloc()->init(p_label_name, NS::UTF8StringEncoding));
	switch (type) {
		case MDCommandBufferStateType::None:
			command_buffer->pushDebugGroup(s.get());
			break;
		case MDCommandBufferStateType::Render:
			render.encoder->pushDebugGroup(s.get());
			break;
		case MDCommandBufferStateType::InlineRender:
			inline_render.encoder->pushDebugGroup(s.get());
			break;
		case MDCommandBufferStateType::Compute:
			compute.encoder->pushDebugGroup(s.get());
			break;
		case MDCommandBufferStateType::Blit:
			blit.encoder->pushDebugGroup(s.get());
			break;
	}
	label_stack.push_back({ type, false });
}

void MDCommandBuffer::end_label() {
	if (label_stack.is_empty()) {
		return;
	}
	LabelStackEntry entry = label_stack[label_stack.size() - 1];
	label_stack.remove_at(label_stack.size() - 1);
	if (entry.stale) {
		return;
	}
	switch (entry.type) {
		case MDCommandBufferStateType::None:
			command_buffer->popDebugGroup();
			break;
		case MDCommandBufferStateType::Render:
			render.encoder->popDebugGroup();
			break;
		case MDCommandBufferStateType::InlineRender:
			inline_render.encoder->popDebugGroup();
			break;
		case MDCommandBufferStateType::Compute:
			compute.encoder->popDebugGroup();
			break;
		case MDCommandBufferStateType::Blit:
			blit.encoder->popDebugGroup();
			break;
	}
}

void MDCommandBuffer::_pop_active_encoder_labels() {
	if (type == MDCommandBufferStateType::None) {
		return;
	}
	for (int64_t i = (int64_t)label_stack.size() - 1; i >= 0; i--) {
		LabelStackEntry &entry = label_stack[i];
		if (entry.stale || entry.type != type) {
			continue;
		}
		switch (type) {
			case MDCommandBufferStateType::Render:
				render.encoder->popDebugGroup();
				break;
			case MDCommandBufferStateType::InlineRender:
				inline_render.encoder->popDebugGroup();
				break;
			case MDCommandBufferStateType::Compute:
				compute.encoder->popDebugGroup();
				break;
			case MDCommandBufferStateType::Blit:
				blit.encoder->popDebugGroup();
				break;
			case MDCommandBufferStateType::None:
				break;
		}
		entry.stale = true;
	}
}

void MDCommandBuffer::_begin() {
	DEV_ASSERT(!state_begin);
	state_begin = true;
	inline_render.reset();

	allocator.get()->reset();
	_scratch.reset();
	release_resources();

	command_buffer->beginCommandBuffer(allocator.get());
}

void MDCommandBuffer::_end() {
	switch (type) {
		case MDCommandBufferStateType::None:
			return;
		case MDCommandBufferStateType::Render:
			return render_end_pass();
		case MDCommandBufferStateType::InlineRender:
			return _end_inline_render();
		case MDCommandBufferStateType::Compute:
			return _end_compute_dispatch();
		case MDCommandBufferStateType::Blit:
			return _end_blit();
	}
}

void MDCommandBuffer::_commit() {
	end();

	render.residency_set = nullptr;
	compute.residency_set = nullptr;

	command_buffer->endCommandBuffer();
	state_begin = false;
}

// The stage mask must be a valid combination for the encoder's own type, so
// each encoder type gets its own mask. Metal 4 folds blit into the unified
// compute encoder, hence Dispatch | Blit for compute. Within a type the mask is
// conservative: the graph does not tell us which stage of a pass reads or
// writes.
static constexpr MTL::Stages RENDER_FENCE_STAGES = MTL::StageVertex | MTL::StageFragment;
static constexpr MTL::Stages COMPUTE_FENCE_STAGES = MTL::StageDispatch | MTL::StageBlit;

void MDCommandBuffer::_fence_wait(MTL4::RenderCommandEncoder *p_enc) {
	MTL::Fence *fence = _fence_to_wait();
	if (p_enc && fence) {
		p_enc->waitForFence(fence, RENDER_FENCE_STAGES);
	}
}

void MDCommandBuffer::_fence_wait(MTL4::ComputeCommandEncoder *p_enc) {
	MTL::Fence *fence = _fence_to_wait();
	if (p_enc && fence) {
		p_enc->waitForFence(fence, COMPUTE_FENCE_STAGES);
	}
}

void MDCommandBuffer::_fence_update(MTL4::RenderCommandEncoder *p_enc) {
	if (!p_enc) {
		return;
	}
	if (MTL::Fence *fence = _fence_to_update()) {
		p_enc->updateFence(fence, RENDER_FENCE_STAGES);
	}
}

void MDCommandBuffer::_fence_update(MTL4::ComputeCommandEncoder *p_enc) {
	if (!p_enc) {
		return;
	}
	if (MTL::Fence *fence = _fence_to_update()) {
		p_enc->updateFence(fence, COMPUTE_FENCE_STAGES);
	}
}

void MDCommandBuffer::pipeline_barrier(BitField<RDD::PipelineStageBits> p_src_stages,
		BitField<RDD::PipelineStageBits> p_dst_stages,
		VectorView<RDD::MemoryAccessBarrier> p_memory_barriers,
		VectorView<RDD::BufferBarrier> p_buffer_barriers,
		VectorView<RDD::TextureBarrier> p_texture_barriers,
		VectorView<RDD::AccelerationStructureBarrier> p_acceleration_structure_barriers) {
	// Nothing to do. Ordering between levels is carried by the level fences,
	// which are driven by command_group_begin/command_group_end. The only live
	// caller for Metal is the render graph's _group_barriers_for_render_commands;
	// the transfer-worker calls in rendering_device.cpp are gated on
	// API_TRAIT_TEXTURES_REQUIRE_LAYOUT_TRANSITIONS, which Metal returns false
	// for, and RenderingDevice exposes no public barrier API.
}

void MDCommandBuffer::bind_pipeline(RDD::PipelineID p_pipeline) {
	MDPipeline *p = (MDPipeline *)(p_pipeline.id);

	// End current encoder if it doesn't match the incoming pipeline type.
	if (type == MDCommandBufferStateType::InlineRender) {
		_end_inline_render();
	} else if (type == MDCommandBufferStateType::Blit) {
		_end_blit();
	}

	if (p->type == MDPipelineType::Render) {
		DEV_ASSERT(type == MDCommandBufferStateType::Render);
		MDRenderPipeline *rp = (MDRenderPipeline *)p;

		if (!render.encoder) {
			// This error would happen if the render pass failed.
			ERR_FAIL_NULL_MSG(render.desc.get(), "Render pass descriptor is null.");

			// This condition occurs when there are no attachments when calling render_next_subpass()
			// and is due to the SUPPORTS_FRAGMENT_SHADER_WITH_ONLY_SIDE_EFFECTS flag.
			render.desc->setDefaultRasterSampleCount(static_cast<NS::UInteger>(rp->sample_count));

			render.encoder = NS::RetainPtr(command_buffer->renderCommandEncoder(render.desc.get()));
			render.encoder_raster = MDRenderPipeline::RasterState();
			_fence_wait(render.encoder.get());
		}

		if (render.pipeline != rp) {
			render.dirty.set_flag((RenderState::DirtyFlag)(RenderState::DIRTY_PIPELINE | RenderState::DIRTY_RASTER));
			// Mark all uniforms as dirty, as variants of a shader pipeline may have a different entry point ABI,
			// due to setting force_active_argument_buffer_resources = true for spirv_cross::CompilerMSL::Options.
			// As a result, uniform sets with the same layout will generate redundant binding warnings when
			// capturing a Metal frame in Xcode.
			//
			// If we don't mark as dirty, then some bindings will generate a validation error.
			render.mark_uniforms_dirty();
			if (render.pipeline != nullptr && render.pipeline->depth_stencil != rp->depth_stencil) {
				render.dirty.set_flag(RenderState::DIRTY_DEPTH);
			}
			if (rp->raster_state.blend.enabled) {
				render.dirty.set_flag(RenderState::DIRTY_BLEND);
			}
			render.pipeline = rp;
		}
	} else if (p->type == MDPipelineType::Compute) {
		DEV_ASSERT(type == MDCommandBufferStateType::Compute);

		if (compute.pipeline != p) {
			compute.dirty.set_flag(ComputeState::DIRTY_PIPELINE);
			compute.mark_uniforms_dirty();
			compute.pipeline = (MDComputePipeline *)p;
		}
	}
}

void MDCommandBuffer::mark_push_constants_dirty() {
	switch (type) {
		case MDCommandBufferStateType::Render:
			render.dirty.set_flag(RenderState::DirtyFlag::DIRTY_PUSH);
			break;
		case MDCommandBufferStateType::Compute:
			compute.dirty.set_flag(ComputeState::DirtyFlag::DIRTY_PUSH);
			break;
		default:
			break;
	}
}

MTL4::ComputeCommandEncoder *MDCommandBuffer::_ensure_blit_encoder() {
	switch (type) {
		case MDCommandBufferStateType::None:
			break;
		case MDCommandBufferStateType::Render:
			render_end_pass();
			break;
		case MDCommandBufferStateType::InlineRender:
			_end_inline_render();
			break;
		case MDCommandBufferStateType::Compute:
			compute_end_pass();
			break;
		case MDCommandBufferStateType::Blit:
			return blit.encoder.get();
	}

	type = MDCommandBufferStateType::Blit;
	blit.encoder = NS::RetainPtr(command_buffer->computeCommandEncoder());
	_fence_wait(blit.encoder.get());
	return blit.encoder.get();
}

void MDCommandBuffer::resolve_texture(RDD::TextureID p_src_texture, RDD::TextureLayout p_src_texture_layout, uint32_t p_src_layer, uint32_t p_src_mipmap, RDD::TextureID p_dst_texture, RDD::TextureLayout p_dst_texture_layout, uint32_t p_dst_layer, uint32_t p_dst_mipmap) {
	MTL::Texture *src_tex = rid::get<RDM::TextureInfo>(p_src_texture)->texture.get();
	MTL::Texture *dst_tex = rid::get<RDM::TextureInfo>(p_dst_texture)->texture.get();

	NS::SharedPtr<MTL4::RenderPassDescriptor> mtlRPD = NS::TransferPtr(MTL4::RenderPassDescriptor::alloc()->init());
	MTL::RenderPassColorAttachmentDescriptor *mtlColorAttDesc = mtlRPD->colorAttachments()->object(0);
	mtlColorAttDesc->setLoadAction(MTL::LoadActionLoad);
	mtlColorAttDesc->setStoreAction(MTL::StoreActionMultisampleResolve);

	mtlColorAttDesc->setTexture(src_tex);
	mtlColorAttDesc->setResolveTexture(dst_tex);
	mtlColorAttDesc->setLevel(p_src_mipmap);
	mtlColorAttDesc->setSlice(p_src_layer);
	mtlColorAttDesc->setResolveLevel(p_dst_mipmap);
	mtlColorAttDesc->setResolveSlice(p_dst_layer);
	MTL4::RenderCommandEncoder *enc = get_new_render_encoder_with_descriptor(mtlRPD.get());
	enc->setLabel(MTLSTR("Resolve Image"));
	_set_inline_render_encoder(enc);
}

void MDCommandBuffer::clear_color_texture(RDD::TextureID p_texture, RDD::TextureLayout p_texture_layout, const Color &p_color, const RDD::TextureSubresourceRange &p_subresources) {
	MTL::Texture *src_tex = rid::get<RDM::TextureInfo>(p_texture)->texture.get();

	if (src_tex->parentTexture()) {
		// Clear via the parent texture rather than the view.
		src_tex = src_tex->parentTexture();
	}

	PixelFormats &pf = device_driver->get_pixel_formats();

	if (pf.isDepthFormat(src_tex->pixelFormat()) || pf.isStencilFormat(src_tex->pixelFormat())) {
		ERR_FAIL_MSG("invalid: depth or stencil texture format");
	}

	NS::SharedPtr<MTL4::RenderPassDescriptor> desc = NS::TransferPtr(MTL4::RenderPassDescriptor::alloc()->init());

	if (p_subresources.aspect.has_flag(RDD::TEXTURE_ASPECT_COLOR_BIT)) {
		MTL::RenderPassColorAttachmentDescriptor *caDesc = desc->colorAttachments()->object(0);
		caDesc->setTexture(src_tex);
		caDesc->setLoadAction(MTL::LoadActionClear);
		caDesc->setStoreAction(MTL::StoreActionStore);
		caDesc->setClearColor(MTL::ClearColor(p_color.r, p_color.g, p_color.b, p_color.a));

		// Extract the mipmap levels that are to be updated.
		uint32_t mipLvlStart = p_subresources.base_mipmap;
		uint32_t mipLvlCnt = p_subresources.mipmap_count;
		uint32_t mipLvlEnd = mipLvlStart + mipLvlCnt;

		NS::UInteger levelCount = src_tex->mipmapLevelCount();

		// Extract the cube or array layers (slices) that are to be updated.
		bool is3D = src_tex->textureType() == MTL::TextureType3D;
		uint32_t layerStart = is3D ? 0 : p_subresources.base_layer;
		uint32_t layerCnt = p_subresources.layer_count;
		uint32_t layerEnd = layerStart + layerCnt;

		const MetalFeatures &features = device_driver->get_device_properties().features;

		// Iterate across mipmap levels and layers, and perform an empty render to clear each.
		for (uint32_t mipLvl = mipLvlStart; mipLvl < mipLvlEnd; mipLvl++) {
			ERR_FAIL_INDEX_MSG(mipLvl, levelCount, "mip level out of range");

			caDesc->setLevel(mipLvl);

			// If a 3D image, we need to get the depth for each level.
			if (is3D) {
				layerCnt = mipmapLevelSizeFromTexture(src_tex, mipLvl).depth;
				layerEnd = layerStart + layerCnt;
			}

			if ((features.layeredRendering && src_tex->sampleCount() == 1) || features.multisampleLayeredRendering) {
				// We can clear all layers at once.
				if (is3D) {
					caDesc->setDepthPlane(layerStart);
				} else {
					caDesc->setSlice(layerStart);
				}
				desc->setRenderTargetArrayLength(layerCnt);
				MTL4::RenderCommandEncoder *enc = get_new_render_encoder_with_descriptor(desc.get());
				enc->setLabel(MTLSTR("Clear Image"));
				if (mipLvl + 1 == mipLvlEnd) {
					_set_inline_render_encoder(enc);
				} else {
					_fence_update(enc);
					enc->endEncoding();
				}
			} else {
				for (uint32_t layer = layerStart; layer < layerEnd; layer++) {
					if (is3D) {
						caDesc->setDepthPlane(layer);
					} else {
						caDesc->setSlice(layer);
					}
					MTL4::RenderCommandEncoder *enc = get_new_render_encoder_with_descriptor(desc.get());
					enc->setLabel(MTLSTR("Clear Image"));
					if (mipLvl + 1 == mipLvlEnd && layer + 1 == layerEnd) {
						_set_inline_render_encoder(enc);
					} else {
						_fence_update(enc);
						enc->endEncoding();
					}
				}
			}
		}
	}
}

void MDCommandBuffer::clear_buffer(RDD::BufferID p_buffer, uint64_t p_offset, uint64_t p_size) {
	MTL4::ComputeCommandEncoder *blit_enc = _ensure_blit_encoder();
	const RDM::BufferInfo *buffer = (const RDM::BufferInfo *)p_buffer.id;

	blit_enc->fillBuffer(buffer->buffer.get(), NS::Range(p_offset, p_size), 0);
}

void MDCommandBuffer::clear_depth_stencil_texture(RDD::TextureID p_texture, RDD::TextureLayout p_texture_layout, float p_depth, uint8_t p_stencil, const RDD::TextureSubresourceRange &p_subresources) {
	MTL::Texture *src_tex = rid::get<RDM::TextureInfo>(p_texture)->texture.get();

	if (src_tex->parentTexture()) {
		// Clear via the parent texture rather than the view.
		src_tex = src_tex->parentTexture();
	}

	PixelFormats &pf = device_driver->get_pixel_formats();

	bool is_depth_format = pf.isDepthFormat(src_tex->pixelFormat());
	bool is_stencil_format = pf.isStencilFormat(src_tex->pixelFormat());

	if (!is_depth_format && !is_stencil_format) {
		ERR_FAIL_MSG("invalid: color texture format");
	}

	bool clear_depth = is_depth_format && p_subresources.aspect.has_flag(RDD::TEXTURE_ASPECT_DEPTH_BIT);
	bool clear_stencil = is_stencil_format && p_subresources.aspect.has_flag(RDD::TEXTURE_ASPECT_STENCIL_BIT);

	if (clear_depth || clear_stencil) {
		NS::SharedPtr<MTL4::RenderPassDescriptor> desc = NS::TransferPtr(MTL4::RenderPassDescriptor::alloc()->init());

		MTL::RenderPassDepthAttachmentDescriptor *daDesc = desc->depthAttachment();
		if (clear_depth) {
			daDesc->setTexture(src_tex);
			daDesc->setLoadAction(MTL::LoadActionClear);
			daDesc->setStoreAction(MTL::StoreActionStore);
			daDesc->setClearDepth(p_depth);
		}

		MTL::RenderPassStencilAttachmentDescriptor *saDesc = desc->stencilAttachment();
		if (clear_stencil) {
			saDesc->setTexture(src_tex);
			saDesc->setLoadAction(MTL::LoadActionClear);
			saDesc->setStoreAction(MTL::StoreActionStore);
			saDesc->setClearStencil(p_stencil);
		}

		// Extract the mipmap levels that are to be updated.
		uint32_t mipLvlStart = p_subresources.base_mipmap;
		uint32_t mipLvlCnt = p_subresources.mipmap_count;
		uint32_t mipLvlEnd = mipLvlStart + mipLvlCnt;

		uint32_t levelCount = src_tex->mipmapLevelCount();

		// Extract the cube or array layers (slices) that are to be updated.
		bool is3D = src_tex->textureType() == MTL::TextureType3D;
		uint32_t layerStart = is3D ? 0 : p_subresources.base_layer;
		uint32_t layerCnt = p_subresources.layer_count;
		uint32_t layerEnd = layerStart + layerCnt;

		const MetalFeatures &features = device_driver->get_device_properties().features;

		// Iterate across mipmap levels and layers, and perform and empty render to clear each.
		for (uint32_t mipLvl = mipLvlStart; mipLvl < mipLvlEnd; mipLvl++) {
			ERR_FAIL_INDEX_MSG(mipLvl, levelCount, "mip level out of range");

			if (clear_depth) {
				daDesc->setLevel(mipLvl);
			}
			if (clear_stencil) {
				saDesc->setLevel(mipLvl);
			}

			// If a 3D image, we need to get the depth for each level.
			if (is3D) {
				layerCnt = mipmapLevelSizeFromTexture(src_tex, mipLvl).depth;
				layerEnd = layerStart + layerCnt;
			}

			if ((features.layeredRendering && src_tex->sampleCount() == 1) || features.multisampleLayeredRendering) {
				// We can clear all layers at once.
				if (is3D) {
					if (clear_depth) {
						daDesc->setDepthPlane(layerStart);
					}
					if (clear_stencil) {
						saDesc->setDepthPlane(layerStart);
					}
				} else {
					if (clear_depth) {
						daDesc->setSlice(layerStart);
					}
					if (clear_stencil) {
						saDesc->setSlice(layerStart);
					}
				}
				desc->setRenderTargetArrayLength(layerCnt);
				MTL4::RenderCommandEncoder *enc = get_new_render_encoder_with_descriptor(desc.get());
				enc->setLabel(MTLSTR("Clear Image"));
				if (mipLvl + 1 == mipLvlEnd) {
					_set_inline_render_encoder(enc);
				} else {
					_fence_update(enc);
					enc->endEncoding();
				}
			} else {
				for (uint32_t layer = layerStart; layer < layerEnd; layer++) {
					if (is3D) {
						if (clear_depth) {
							daDesc->setDepthPlane(layer);
						}
						if (clear_stencil) {
							saDesc->setDepthPlane(layer);
						}
					} else {
						if (clear_depth) {
							daDesc->setSlice(layer);
						}
						if (clear_stencil) {
							saDesc->setSlice(layer);
						}
					}
					MTL4::RenderCommandEncoder *enc = get_new_render_encoder_with_descriptor(desc.get());
					enc->setLabel(MTLSTR("Clear Image"));
					if (mipLvl + 1 == mipLvlEnd && layer + 1 == layerEnd) {
						_set_inline_render_encoder(enc);
					} else {
						_fence_update(enc);
						enc->endEncoding();
					}
				}
			}
		}
	}
}

void MDCommandBuffer::copy_buffer(RDD::BufferID p_src_buffer, RDD::BufferID p_dst_buffer, VectorView<RDD::BufferCopyRegion> p_regions) {
	const RDM::BufferInfo *src = (const RDM::BufferInfo *)p_src_buffer.id;
	const RDM::BufferInfo *dst = (const RDM::BufferInfo *)p_dst_buffer.id;

	MTL4::ComputeCommandEncoder *enc = _ensure_blit_encoder();

	for (uint32_t i = 0; i < p_regions.size(); i++) {
		RDD::BufferCopyRegion region = p_regions[i];
		enc->copyFromBuffer(src->buffer.get(), region.src_offset, dst->buffer.get(), region.dst_offset, region.size);
	}
}

void MDCommandBuffer::copy_texture(RDD::TextureID p_src_texture, RDD::TextureID p_dst_texture, VectorView<RDD::TextureCopyRegion> p_regions) {
	MTL::Texture *src = rid::get<RDM::TextureInfo>(p_src_texture)->texture.get();
	MTL::Texture *dst = rid::get<RDM::TextureInfo>(p_dst_texture)->texture.get();

	MTL4::ComputeCommandEncoder *enc = _ensure_blit_encoder();
	PixelFormats &pf = device_driver->get_pixel_formats();

	MTL::PixelFormat src_fmt = src->pixelFormat();
	bool src_is_compressed = pf.getFormatType(src_fmt) == MTLFormatType::Compressed;
	MTL::PixelFormat dst_fmt = dst->pixelFormat();
	bool dst_is_compressed = pf.getFormatType(dst_fmt) == MTLFormatType::Compressed;

	// Validate copy.
	if (src->sampleCount() != dst->sampleCount() || pf.getBytesPerBlock(src_fmt) != pf.getBytesPerBlock(dst_fmt)) {
		ERR_FAIL_MSG("Cannot copy between incompatible pixel formats, such as formats of different pixel sizes, or between images with different sample counts.");
	}

	// If source and destination have different formats and at least one is compressed, a temporary buffer is required.
	bool need_tmp_buffer = (src_fmt != dst_fmt) && (src_is_compressed || dst_is_compressed);
	if (need_tmp_buffer) {
		ERR_FAIL_MSG("not implemented: copy with intermediate buffer");
	}

	NS::SharedPtr<MTL::Texture> src_view;
	if (src_fmt != dst_fmt) {
		// Map the source pixel format to the dst through a texture view on the source texture.
		src_view = NS::TransferPtr(src->newTextureView(dst_fmt));
		src = src_view.get();
	}

	for (uint32_t i = 0; i < p_regions.size(); i++) {
		RDD::TextureCopyRegion region = p_regions[i];

		MTL::Size extent = MTLSizeFromVector3i(region.size);

		// If copies can be performed using direct texture-texture copying, do so.
		uint32_t src_level = region.src_subresources.mipmap;
		uint32_t src_base_layer = region.src_subresources.base_layer;
		MTL::Size src_extent = mipmapLevelSizeFromTexture(src, src_level);
		uint32_t dst_level = region.dst_subresources.mipmap;
		uint32_t dst_base_layer = region.dst_subresources.base_layer;
		MTL::Size dst_extent = mipmapLevelSizeFromTexture(dst, dst_level);

		// All layers may be copied at once, if the extent completely covers both images.
		if (src_extent == extent && dst_extent == extent) {
			enc->copyFromTexture(src, src_base_layer, src_level,
					dst, dst_base_layer, dst_level,
					region.src_subresources.layer_count, 1);
		} else {
			MTL::Origin src_origin = MTLOriginFromVector3i(region.src_offset);
			MTL::Size src_size = clampMTLSize(extent, src_origin, src_extent);
			uint32_t layer_count = 0;
			if ((src->textureType() == MTL::TextureType3D) != (dst->textureType() == MTL::TextureType3D)) {
				// In the case, the number of layers to copy is in extent.depth. Use that value,
				// then clamp the depth, so we don't try to copy more than Metal will allow.
				layer_count = extent.depth;
				src_size.depth = 1;
			} else {
				layer_count = region.src_subresources.layer_count;
			}
			MTL::Origin dst_origin = MTLOriginFromVector3i(region.dst_offset);

			for (uint32_t layer = 0; layer < layer_count; layer++) {
				// We can copy between a 3D and a 2D image easily. Just copy between
				// one slice of the 2D image and one plane of the 3D image at a time.
				if ((src->textureType() == MTL::TextureType3D) == (dst->textureType() == MTL::TextureType3D)) {
					enc->copyFromTexture(src, src_base_layer + layer, src_level, src_origin, src_size,
							dst, dst_base_layer + layer, dst_level, dst_origin);
				} else if (src->textureType() == MTL::TextureType3D) {
					enc->copyFromTexture(src, src_base_layer, src_level,
							MTL::Origin(src_origin.x, src_origin.y, src_origin.z + layer), src_size,
							dst, dst_base_layer + layer, dst_level, dst_origin);
				} else {
					DEV_ASSERT(dst->textureType() == MTL::TextureType3D);
					enc->copyFromTexture(src, src_base_layer + layer, src_level, src_origin, src_size,
							dst, dst_base_layer,
							dst_level, MTL::Origin(dst_origin.x, dst_origin.y, dst_origin.z + layer));
				}
			}
		}
	}
}

void MDCommandBuffer::copy_buffer_to_texture(RDD::BufferID p_src_buffer, RDD::TextureID p_dst_texture, VectorView<RDD::BufferTextureCopyRegion> p_regions) {
	_copy_texture_buffer(CopySource::Buffer, p_dst_texture, p_src_buffer, p_regions);
}

void MDCommandBuffer::copy_texture_to_buffer(RDD::TextureID p_src_texture, RDD::BufferID p_dst_buffer, VectorView<RDD::BufferTextureCopyRegion> p_regions) {
	_copy_texture_buffer(CopySource::Texture, p_src_texture, p_dst_buffer, p_regions);
}

void MDCommandBuffer::_copy_texture_buffer(CopySource p_source,
		RDD::TextureID p_texture,
		RDD::BufferID p_buffer,
		VectorView<RDD::BufferTextureCopyRegion> p_regions) {
	const RDM::BufferInfo *buffer = (const RDM::BufferInfo *)p_buffer.id;
	MTL::Texture *texture = rid::get<RDM::TextureInfo>(p_texture)->texture.get();

	MTL4::ComputeCommandEncoder *enc = _ensure_blit_encoder();

	PixelFormats &pf = device_driver->get_pixel_formats();
	MTL::PixelFormat mtlPixFmt = texture->pixelFormat();

	MTL::BlitOption options = MTL::BlitOptionNone;
	if (pf.isPVRTCFormat(mtlPixFmt)) {
		options |= MTL::BlitOptionRowLinearPVRTC;
	}

	for (uint32_t i = 0; i < p_regions.size(); i++) {
		RDD::BufferTextureCopyRegion region = p_regions[i];

		uint32_t mip_level = region.texture_subresource.mipmap;
		MTL::Origin txt_origin = MTL::Origin(region.texture_offset.x, region.texture_offset.y, region.texture_offset.z);
		MTL::Size src_extent = mipmapLevelSizeFromTexture(texture, mip_level);
		MTL::Size txt_size = clampMTLSize(MTL::Size(region.texture_region_size.x, region.texture_region_size.y, region.texture_region_size.z),
				txt_origin,
				src_extent);

		uint32_t buffImgWd = region.texture_region_size.x;
		uint32_t buffImgHt = region.texture_region_size.y;

		NS::UInteger bytesPerRow = pf.getBytesPerRow(mtlPixFmt, buffImgWd);
		NS::UInteger bytesPerImg = pf.getBytesPerLayer(mtlPixFmt, bytesPerRow, buffImgHt);

		MTL::BlitOption blit_options = options;

		if (pf.isDepthFormat(mtlPixFmt) && pf.isStencilFormat(mtlPixFmt)) {
			// Don't reduce depths of 32-bit depth/stencil formats.
			if (region.texture_subresource.aspect == RDD::TEXTURE_ASPECT_DEPTH) {
				if (pf.getBytesPerTexel(mtlPixFmt) != 4) {
					bytesPerRow -= buffImgWd;
					bytesPerImg -= buffImgWd * buffImgHt;
				}
				blit_options |= MTL::BlitOptionDepthFromDepthStencil;
			} else if (region.texture_subresource.aspect == RDD::TEXTURE_ASPECT_STENCIL) {
				// The stencil component is always 1 byte per pixel.
				bytesPerRow = buffImgWd;
				bytesPerImg = buffImgWd * buffImgHt;
				blit_options |= MTL::BlitOptionStencilFromDepthStencil;
			}
		}

		if (!isArrayTexture(texture->textureType())) {
			bytesPerImg = 0;
		}

		if (p_source == CopySource::Buffer) {
			enc->copyFromBuffer(buffer->buffer.get(), region.buffer_offset,
					bytesPerRow, bytesPerImg, txt_size,
					texture, region.texture_subresource.layer, mip_level, txt_origin, blit_options);
		} else {
			enc->copyFromTexture(texture, region.texture_subresource.layer, mip_level,
					txt_origin, txt_size,
					buffer->buffer.get(), region.buffer_offset,
					bytesPerRow, bytesPerImg, blit_options);
		}
	}
}

MTL4::RenderCommandEncoder *MDCommandBuffer::get_new_render_encoder_with_descriptor(MTL4::RenderPassDescriptor *p_desc) {
	switch (type) {
		case MDCommandBufferStateType::None:
			break;
		case MDCommandBufferStateType::Render:
			render_end_pass();
			break;
		case MDCommandBufferStateType::InlineRender:
			_end_inline_render();
			break;
		case MDCommandBufferStateType::Compute:
			_end_compute_dispatch();
			break;
		case MDCommandBufferStateType::Blit:
			_end_blit();
			break;
	}

	MTL4::RenderCommandEncoder *enc = command_buffer->renderCommandEncoder(p_desc);
	_fence_wait(enc);
	return enc;
}

#pragma mark - Render Commands

void MDCommandBuffer::render_bind_uniform_sets(VectorView<RDD::UniformSetID> p_uniform_sets, RDD::ShaderID p_shader, uint32_t p_first_set_index, uint32_t p_set_count, uint32_t p_dynamic_offsets) {
	DEV_ASSERT(type == MDCommandBufferStateType::Render);

	if (uint32_t new_size = p_first_set_index + p_set_count; render.uniform_sets.size() < new_size) {
		uint32_t s = render.uniform_sets.size();
		render.uniform_sets.resize(new_size);
		// Set intermediate values to null.
		std::fill(&render.uniform_sets[s], render.uniform_sets.end().operator->(), nullptr);
	}

	const MDShader *shader = (const MDShader *)p_shader.id;
	DynamicOffsetLayout layout = shader->dynamic_offset_layout;

	// Clear bits for sets being bound, then OR new values.
	for (uint32_t i = 0; i < p_set_count && render.dynamic_offsets != 0; i++) {
		uint32_t set_index = p_first_set_index + i;
		uint32_t count = layout.get_count(set_index);
		if (count > 0) {
			uint32_t shift = layout.get_offset_index_shift(set_index);
			uint32_t mask = ((1u << (count * 4u)) - 1u) << shift;
			render.dynamic_offsets &= ~mask; // Clear this set's bits
		}
	}
	render.dynamic_offsets |= p_dynamic_offsets;

	for (size_t i = 0; i < p_set_count; ++i) {
		MDUniformSet *set = (MDUniformSet *)(p_uniform_sets[i].id);

		uint32_t index = p_first_set_index + i;
		if (render.uniform_sets[index] != set || layout.get_count(index) > 0) {
			render.dirty.set_flag(RenderState::DIRTY_UNIFORMS);
			render.uniform_set_mask |= 1ULL << index;
			render.uniform_sets[index] = set;
		}
	}
}

void MDCommandBuffer::render_clear_attachments(VectorView<RDD::AttachmentClear> p_attachment_clears, VectorView<Rect2i> p_rects) {
	DEV_ASSERT(type == MDCommandBufferStateType::Render);

	const MDSubpass &subpass = render.get_subpass();

	uint32_t vertex_count = p_rects.size() * 6 * subpass.view_count;
	simd::float4 *vertices = ALLOCA_ARRAY(simd::float4, vertex_count);
	simd::float4 clear_colors[ClearAttKey::ATTACHMENT_COUNT];

	Size2i size = render.frameBuffer->size;
	Rect2i render_area = render.clip_to_render_area({ { 0, 0 }, size });
	size = Size2i(render_area.position.x + render_area.size.width, render_area.position.y + render_area.size.height);
	_populate_vertices(vertices, size, p_rects);

	ClearAttKey key;
	key.sample_count = render.pass->get_sample_count();
	if (subpass.view_count > 1) {
		key.enable_layered_rendering();
	}

	float depth_value = 0;
	uint32_t stencil_value = 0;

	for (uint32_t i = 0; i < p_attachment_clears.size(); i++) {
		const RDD::AttachmentClear &attClear = p_attachment_clears[i];
		uint32_t attachment_index;
		if (attClear.aspect.has_flag(RDD::TEXTURE_ASPECT_COLOR_BIT)) {
			attachment_index = attClear.color_attachment;
		} else {
			attachment_index = subpass.depth_stencil_reference.attachment;
		}

		const MDAttachment &mda = render.pass->attachments[attachment_index];
		if (attClear.aspect.has_flag(RDD::TEXTURE_ASPECT_COLOR_BIT)) {
			key.set_color_format(attachment_index, mda.format);
			clear_colors[attachment_index] = simd_make_float4(
					attClear.value.color.r,
					attClear.value.color.g,
					attClear.value.color.b,
					attClear.value.color.a);
		}

		if (attClear.aspect.has_flag(RDD::TEXTURE_ASPECT_DEPTH_BIT)) {
			key.set_depth_format(mda.format);
			depth_value = attClear.value.depth;
		}

		if (attClear.aspect.has_flag(RDD::TEXTURE_ASPECT_STENCIL_BIT)) {
			key.set_stencil_format(mda.format);
			stencil_value = attClear.value.stencil;
		}
	}
	clear_colors[ClearAttKey::DEPTH_INDEX] = simd_make_float4(depth_value);

	MTL4::RenderCommandEncoder *enc = render.encoder.get();

	MDResourceCache &cache = device_driver->get_resource_cache();

	enc->pushDebugGroup(MTLSTR("ClearAttachments"));
	enc->setRenderPipelineState(cache.get_clear_render_pipeline_state(key, nullptr));
	enc->setDepthStencilState(cache.get_depth_stencil_state(
			key.is_depth_enabled(),
			key.is_stencil_enabled()));
	enc->setStencilReferenceValue(stencil_value);
	enc->setCullMode(MTL::CullModeNone);
	enc->setTriangleFillMode(MTL::TriangleFillModeFill);
	enc->setDepthBias(0, 0, 0);
	enc->setViewport(MTL::Viewport{ 0, 0, (double)size.width, (double)size.height, 0.0, 1.0 });
	enc->setScissorRect(MTL::ScissorRect{ 0, 0, (NS::UInteger)size.width, (NS::UInteger)size.height });

	{
		MDRingBuffer::Allocation dst = _scratch.allocate(sizeof(clear_colors));
		memcpy(dst.ptr, clear_colors, sizeof(clear_colors));
		_args_clear.get()->setAddress(dst.gpu_address, 0);
	}

	{
		uint32_t size = vertex_count * sizeof(vertices[0]);
		MDRingBuffer::Allocation dst = _scratch.allocate(size);
		memcpy(dst.ptr, vertices, size);
		_args_clear.get()->setAddress(dst.gpu_address, device_driver->get_metal_buffer_index_for_vertex_attribute_binding(VERT_CONTENT_BUFFER_INDEX));
	}

	enc->setArgumentTable(_args_clear.get(), MTL::RenderStageVertex | MTL::RenderStageFragment);

	enc->drawPrimitives(MTL::PrimitiveTypeTriangle, (NS::UInteger)0, vertex_count);
	enc->popDebugGroup();

	render.dirty.set_flag((RenderState::DirtyFlag)(RenderState::DIRTY_PIPELINE | RenderState::DIRTY_DEPTH | RenderState::DIRTY_RASTER));
	// Record the raster state written directly above, so the next draw re-encodes only what
	// actually differs. Depth clip mode, winding and blend color were not touched.
	render.encoder_raster.cull_mode = MTL::CullModeNone;
	render.encoder_raster.fill_mode = MTL::TriangleFillModeFill;
	render.encoder_raster.depth_bias = MDRenderPipeline::RasterState::DepthBias();
	render.encoder_raster.stencil.front_reference = stencil_value;
	render.encoder_raster.stencil.back_reference = stencil_value;
	render.mark_uniforms_dirty({ 0 }); // Mark index 0 dirty, if there is already a binding for index 0.
	render.mark_viewport_dirty();
	render.mark_scissors_dirty();
	render.mark_vertex_dirty();
	render.mark_blend_dirty();
}

void MDCommandBuffer::_render_set_dirty_state() {
	_render_bind_uniform_sets();

	if (render.dirty.has_flag(RenderState::DIRTY_PUSH)) {
		// Read the binding from the bound pipeline, not from command buffer state: DIRTY_PUSH
		// is also re-armed by DIRTY_ALL on a new subpass, long after the data was captured, and
		// a stale binding index could overwrite an unrelated argument table slot.
		const MDRenderShader *shader = render.pipeline->shader;
		if (push_constant_shader == shader && push_constant_data_len > 0) {
			MDRingBuffer::Allocation dst = _scratch.allocate(push_constant_data_len);
			memcpy(dst.ptr, &push_constant_data, push_constant_data_len);
			render.args->setAddress(dst.gpu_address, shader->push_constants.binding);
		}
	}

	const MDSubpass &subpass = render.get_subpass();
	if (subpass.view_count > 1) {
		MDRingBuffer::Allocation dst = _scratch.allocate(sizeof(uint32_t) * 2);
		uint32_t *view_range = (uint32_t *)dst.ptr;
		view_range[0] = 0;
		view_range[1] = subpass.view_count;

		render.args->setAddress(dst.gpu_address, VIEW_MASK_BUFFER_INDEX);
	}

	MTL4::RenderCommandEncoder *enc = render.encoder.get();

	if (render.dirty.has_flag(RenderState::DIRTY_PIPELINE)) {
		enc->setRenderPipelineState(render.pipeline->state.get());
	}

	if (render.dirty.has_flag(RenderState::DIRTY_VIEWPORT)) {
		enc->setViewports(reinterpret_cast<const MTL::Viewport *>(render.viewports.ptr()), render.viewports.size());
	}

	if (render.dirty.has_flag(RenderState::DIRTY_DEPTH)) {
		enc->setDepthStencilState(render.pipeline->depth_stencil.get());
	}

	if (render.dirty.has_flag(RenderState::DIRTY_RASTER)) {
		render.pipeline->raster_state.apply(enc, render.encoder_raster);
	}

	if (render.dirty.has_flag(RenderState::DIRTY_SCISSOR) && !render.scissors.is_empty()) {
		size_t len = render.scissors.size();
		MTL::ScissorRect *rects = ALLOCA_ARRAY(MTL::ScissorRect, len);
		for (size_t i = 0; i < len; i++) {
			rects[i] = render.clip_to_render_area(render.scissors[i]);
		}
		enc->setScissorRects(rects, len);
	}

	// The dynamic blend constant overrides the pipeline's static one, which raster_state
	// applied above, so the encoder mirror has to be updated or it stops describing the
	// encoder and a later apply() would compare against a stale baseline.
	if (render.dirty.has_flag(RenderState::DIRTY_BLEND) && render.blend_constants.has_value() &&
			!render.encoder_raster.blend.has_color(*render.blend_constants)) {
		enc->setBlendColor(render.blend_constants->r, render.blend_constants->g, render.blend_constants->b, render.blend_constants->a);
		render.encoder_raster.blend.set_color(*render.blend_constants);
	}

	if (render.dirty.has_flag(RenderState::DIRTY_VERTEX)) {
		uint32_t p_binding_count = render.vertex_buffers.size();
		if (p_binding_count > 0) {
			uint32_t first = device_driver->get_metal_buffer_index_for_vertex_attribute_binding(p_binding_count - 1);

			MTL::Buffer **buf_ptr = render.vertex_buffers.ptr();
			NS::UInteger *ofs_ptr = render.vertex_offsets.ptr();
			for (uint32_t i = first; i < first + p_binding_count; i++) {
				render.args->setAddress((*buf_ptr)->gpuAddress() + *ofs_ptr, i);
				buf_ptr++;
				ofs_ptr++;
			}
		}
	}

	render.dirty.clear();
}

void MDCommandBuffer::_render_bind_uniform_sets() {
	DEV_ASSERT(type == MDCommandBufferStateType::Render);
	if (!render.dirty.has_flag(RenderState::DIRTY_UNIFORMS)) {
		return;
	}

	render.dirty.clear_flag(RenderState::DIRTY_UNIFORMS);
	uint64_t set_uniforms = render.uniform_set_mask;
	render.uniform_set_mask = 0;

	MDRenderShader *shader = render.pipeline->shader;
	const uint32_t dynamic_offsets = render.dynamic_offsets;

	while (set_uniforms != 0) {
		// Find the index of the next set bit.
		uint32_t index = (uint32_t)__builtin_ctzll(set_uniforms);
		// Clear the set bit.
		set_uniforms &= (set_uniforms - 1);
		MDUniformSet *set = render.uniform_sets[index];
		if (set == nullptr || index >= (uint32_t)shader->sets.size()) {
			continue;
		}
		if (shader->uses_argument_buffers) {
			_bind_uniforms_argument_buffers(set, shader, index, dynamic_offsets);
		} else {
			_bind_uniforms_direct(set, shader, render.residency_set.get(), render.args.get(), index, dynamic_offsets);
		}
	}

	render.encoder->setArgumentTable(render.args.get(), MTL::RenderStageVertex | MTL::RenderStageFragment);
}

void MDCommandBuffer::render_begin_pass(RDD::RenderPassID p_render_pass, RDD::FramebufferID p_frameBuffer, RDD::CommandBufferType p_cmd_buffer_type, const Rect2i &p_rect, VectorView<RDD::RenderPassClearValue> p_clear_values) {
	DEV_ASSERT(command_buffer);
	end();

	MDRenderPass *pass = (MDRenderPass *)(p_render_pass.id);
	MDFrameBuffer *fb = (MDFrameBuffer *)(p_frameBuffer.id);

	type = MDCommandBufferStateType::Render;
	render.pass = pass;
	render.current_subpass = nullptr;
	render.render_area = p_rect;
	render.clear_values.resize(p_clear_values.size());
	for (uint32_t i = 0; i < p_clear_values.size(); i++) {
		render.clear_values[i] = p_clear_values[i];
	}
	render.is_rendering_entire_area = (p_rect.position == Point2i(0, 0)) && p_rect.size == fb->size;
	render.frameBuffer = fb;
	render_next_subpass();
}

void MDCommandBuffer::render_next_subpass() {
	DEV_ASSERT(command_buffer);

	MDSubpass *prev_subpass = render.current_subpass;
	// Each subpass gets its own encoder; close the previous one first.
	if (prev_subpass != nullptr) {
		_end_render_pass();
	}

	render.current_subpass = (prev_subpass == nullptr)
			? &render.pass->subpasses[0]
			: prev_subpass + 1;

	const MDFrameBuffer &fb = *render.frameBuffer;
	const MDRenderPass &pass = *render.pass;
	const MDSubpass &subpass = render.get_subpass();

	NS::SharedPtr<MTL4::RenderPassDescriptor> desc = NS::TransferPtr(MTL4::RenderPassDescriptor::alloc()->init());

	if (subpass.view_count > 1) {
		desc->setRenderTargetArrayLength(subpass.view_count);
	}

	PixelFormats &pf = device_driver->get_pixel_formats();

	uint32_t attachmentCount = 0;
	for (uint32_t i = 0; i < subpass.color_references.size(); i++) {
		uint32_t idx = subpass.color_references[i].attachment;
		if (idx == RDD::AttachmentReference::UNUSED) {
			continue;
		}

		attachmentCount += 1;
		MTL::RenderPassColorAttachmentDescriptor *ca = desc->colorAttachments()->object(i);

		uint32_t resolveIdx = subpass.resolve_references.is_empty() ? RDD::AttachmentReference::UNUSED : subpass.resolve_references[i].attachment;
		bool has_resolve = resolveIdx != RDD::AttachmentReference::UNUSED;
		bool can_resolve = true;
		if (resolveIdx != RDD::AttachmentReference::UNUSED) {
			MTL::Texture *resolve_tex = fb.get_texture(resolveIdx);
			can_resolve = flags::all(pf.getCapabilities(resolve_tex->pixelFormat()), kMTLFmtCapsResolve);
			if (can_resolve) {
				ca->setResolveTexture(resolve_tex);
			} else {
				CRASH_NOW_MSG("unimplemented: using a texture format that is not supported for resolve");
			}
		}

		const MDAttachment &attachment = pass.attachments[idx];

		MTL::Texture *tex = fb.get_texture(idx);
		ERR_FAIL_NULL_MSG(tex, "Frame buffer color texture is null.");

		if ((attachment.type & MDAttachmentType::Color)) {
			if (attachment.configureDescriptor(ca, pf, subpass, tex, render.is_rendering_entire_area, has_resolve, can_resolve, false)) {
				Color clearColor = render.clear_values[idx].color;
				ca->setClearColor(MTL::ClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a));
			}
		}
	}

	if (subpass.depth_stencil_reference.attachment != RDD::AttachmentReference::UNUSED) {
		attachmentCount += 1;
		uint32_t idx = subpass.depth_stencil_reference.attachment;
		const MDAttachment &attachment = pass.attachments[idx];
		MTL::Texture *tex = fb.get_texture(idx);
		ERR_FAIL_NULL_MSG(tex, "Frame buffer depth / stencil texture is null.");
		if (attachment.type & MDAttachmentType::Depth) {
			MTL::RenderPassDepthAttachmentDescriptor *da = desc->depthAttachment();
			if (attachment.configureDescriptor(da, pf, subpass, tex, render.is_rendering_entire_area, false, false, false)) {
				da->setClearDepth(render.clear_values[idx].depth);
			}
		}

		if (attachment.type & MDAttachmentType::Stencil) {
			MTL::RenderPassStencilAttachmentDescriptor *sa = desc->stencilAttachment();
			if (attachment.configureDescriptor(sa, pf, subpass, tex, render.is_rendering_entire_area, false, false, true)) {
				sa->setClearStencil(render.clear_values[idx].stencil);
			}
		}
	}

	desc->setRenderTargetWidth(MAX((NS::UInteger)MIN(render.render_area.position.x + render.render_area.size.width, fb.size.width), 1u));
	desc->setRenderTargetHeight(MAX((NS::UInteger)MIN(render.render_area.position.y + render.render_area.size.height, fb.size.height), 1u));

	if (attachmentCount == 0) {
		// If there are no attachments, delay the creation of the encoder,
		// so we can use a matching sample count for the pipeline, by setting
		// the defaultRasterSampleCount from the pipeline's sample count.
		render.desc = desc;
	} else {
		render.encoder = NS::RetainPtr(command_buffer->renderCommandEncoder(desc.get()));
		render.encoder_raster = MDRenderPipeline::RasterState();
		_fence_wait(render.encoder.get());

		if (!render.is_rendering_entire_area) {
			_render_clear_render_area();
		}
		// With a new encoder, all state is dirty.
		render.dirty.set_flag(RenderState::DIRTY_ALL);
	}
}

void MDCommandBuffer::render_draw(uint32_t p_vertex_count,
		uint32_t p_instance_count,
		uint32_t p_base_vertex,
		uint32_t p_first_instance) {
	DEV_ASSERT(type == MDCommandBufferStateType::Render);
	ERR_FAIL_NULL_MSG(render.pipeline, "No pipeline set for render command buffer.");

	_render_set_dirty_state();

	const MDSubpass &subpass = render.get_subpass();
	if (subpass.view_count > 1) {
		p_instance_count *= subpass.view_count;
	}

	DEV_ASSERT(render.dirty == 0);

	MTL4::RenderCommandEncoder *enc = render.encoder.get();
	enc->drawPrimitives(render.pipeline->raster_state.render_primitive, p_base_vertex, p_vertex_count, p_instance_count, p_first_instance);
}

void MDCommandBuffer::render_bind_vertex_buffers(uint32_t p_binding_count, const RDD::BufferID *p_buffers, const uint64_t *p_offsets, uint64_t p_dynamic_offsets) {
	DEV_ASSERT(type == MDCommandBufferStateType::Render);

	render.vertex_buffers.resize(p_binding_count);
	render.vertex_offsets.resize(p_binding_count);

	// Reverse the buffers, as their bindings are assigned in descending order.
	for (uint32_t i = 0; i < p_binding_count; i += 1) {
		const RenderingDeviceDriverMetal::BufferInfo *buf_info = (const RenderingDeviceDriverMetal::BufferInfo *)p_buffers[p_binding_count - i - 1].id;

		NS::UInteger dynamic_offset = 0;
		if (buf_info->is_dynamic()) {
			const MetalBufferDynamicInfo *dyn_buf = (const MetalBufferDynamicInfo *)buf_info;
			uint64_t frame_idx = p_dynamic_offsets & 0x3;
			p_dynamic_offsets >>= 2;
			dynamic_offset = frame_idx * dyn_buf->size_bytes;
		}
		render.vertex_buffers[i] = buf_info->buffer.get();
		render.vertex_offsets[i] = dynamic_offset + p_offsets[p_binding_count - i - 1];
	}

	if (render.encoder) {
		uint32_t first = device_driver->get_metal_buffer_index_for_vertex_attribute_binding(p_binding_count - 1);

		MTL::Buffer **buf_ptr = render.vertex_buffers.ptr();
		NS::UInteger *ofs_ptr = render.vertex_offsets.ptr();
		for (uint32_t i = first; i < first + p_binding_count; i++) {
			render.args->setAddress((*buf_ptr)->gpuAddress() + *ofs_ptr, i);
			buf_ptr++;
			ofs_ptr++;
		}
		render.dirty.clear_flag(RenderState::DIRTY_VERTEX);
	} else {
		render.dirty.set_flag(RenderState::DIRTY_VERTEX);
	}
}

void MDCommandBuffer::render_bind_index_buffer(RDD::BufferID p_buffer, RDD::IndexBufferFormat p_format, uint64_t p_offset) {
	DEV_ASSERT(type == MDCommandBufferStateType::Render);

	const RenderingDeviceDriverMetal::BufferInfo *buffer = (const RenderingDeviceDriverMetal::BufferInfo *)p_buffer.id;

	render.index_buffer = buffer->buffer.get();
	render.index_type = p_format == RDD::IndexBufferFormat::INDEX_BUFFER_FORMAT_UINT16 ? MTL::IndexTypeUInt16 : MTL::IndexTypeUInt32;
	render.index_offset = p_offset;
}

void MDCommandBuffer::render_draw_indexed(uint32_t p_index_count,
		uint32_t p_instance_count,
		uint32_t p_first_index,
		int32_t p_vertex_offset,
		uint32_t p_first_instance) {
	DEV_ASSERT(type == MDCommandBufferStateType::Render);
	ERR_FAIL_NULL_MSG(render.pipeline, "No pipeline set for render command buffer.");

	_render_set_dirty_state();

	const MDSubpass &subpass = render.get_subpass();
	if (subpass.view_count > 1) {
		p_instance_count *= subpass.view_count;
	}

	MTL4::RenderCommandEncoder *enc = render.encoder.get();

	uint32_t index_offset = render.index_offset;
	index_offset += p_first_index * render.index_type_size();

	// TODO(sgc): Verify length is correctly calculated
	enc->drawIndexedPrimitives(render.pipeline->raster_state.render_primitive,
			p_index_count, render.index_type,
			render.index_buffer->gpuAddress() + index_offset,
			render.index_buffer->length() - index_offset,
			p_instance_count, p_vertex_offset, p_first_instance);
}

void MDCommandBuffer::render_draw_indexed_indirect(RDD::BufferID p_indirect_buffer, uint64_t p_offset, uint32_t p_draw_count, uint32_t p_stride) {
	DEV_ASSERT(type == MDCommandBufferStateType::Render);
	ERR_FAIL_NULL_MSG(render.pipeline, "No pipeline set for render command buffer.");

	_render_set_dirty_state();

	MTL4::RenderCommandEncoder *enc = render.encoder.get();

	const RenderingDeviceDriverMetal::BufferInfo *indirect_buf = (const RenderingDeviceDriverMetal::BufferInfo *)p_indirect_buffer.id;
	MTL::Buffer *indirect_buffer = indirect_buf->buffer.get();
	uint64_t indirect_addr = indirect_buffer->gpuAddress() + p_offset;
	uint64_t index_addr = render.index_buffer->gpuAddress();
	NS::UInteger index_length = render.index_buffer->length();

	for (uint32_t i = 0; i < p_draw_count; i++) {
		enc->drawIndexedPrimitives(render.pipeline->raster_state.render_primitive,
				render.index_type, index_addr, index_length, indirect_addr);
		indirect_addr += p_stride;
	}
}

void MDCommandBuffer::render_draw_indexed_indirect_count(RDD::BufferID p_indirect_buffer, uint64_t p_offset, RDD::BufferID p_count_buffer, uint64_t p_count_buffer_offset, uint32_t p_max_draw_count, uint32_t p_stride) {
	ERR_FAIL_MSG("not implemented");
}

void MDCommandBuffer::render_draw_indirect(RDD::BufferID p_indirect_buffer, uint64_t p_offset, uint32_t p_draw_count, uint32_t p_stride) {
	DEV_ASSERT(type == MDCommandBufferStateType::Render);
	ERR_FAIL_NULL_MSG(render.pipeline, "No pipeline set for render command buffer.");

	_render_set_dirty_state();

	MTL4::RenderCommandEncoder *enc = render.encoder.get();

	const RenderingDeviceDriverMetal::BufferInfo *indirect_buf = (const RenderingDeviceDriverMetal::BufferInfo *)p_indirect_buffer.id;
	MTL::Buffer *indirect_buffer = indirect_buf->buffer.get();
	uint64_t indirect_addr = indirect_buffer->gpuAddress() + p_offset;

	for (uint32_t i = 0; i < p_draw_count; i++) {
		enc->drawPrimitives(render.pipeline->raster_state.render_primitive, indirect_addr);
		indirect_addr += p_stride;
	}
}

void MDCommandBuffer::render_draw_indirect_count(RDD::BufferID p_indirect_buffer, uint64_t p_offset, RDD::BufferID p_count_buffer, uint64_t p_count_buffer_offset, uint32_t p_max_draw_count, uint32_t p_stride) {
	ERR_FAIL_MSG("not implemented");
}

void MDCommandBuffer::render_end_pass() {
	DEV_ASSERT(type == MDCommandBufferStateType::Render);

	_pop_active_encoder_labels();
	_fence_update(render.encoder.get());
	render.end_encoding();
	render.reset();
	reset();
}

#pragma mark - RenderState

void MDCommandBuffer::RenderState::reset() {
	pass = nullptr;
	frameBuffer = nullptr;
	pipeline = nullptr;
	current_subpass = nullptr;
	render_area = {};
	is_rendering_entire_area = false;
	desc.reset();
	encoder.reset();
	index_buffer = nullptr;
	index_type = MTL::IndexTypeUInt16;
	dirty = DIRTY_NONE;
	uniform_sets.clear();
	dynamic_offsets = 0;
	uniform_set_mask = 0;
	clear_values.clear();
	viewports.clear();
	scissors.clear();
	blend_constants.reset();
	bzero(vertex_buffers.ptr(), sizeof(MTL::Buffer *) * vertex_buffers.size());
	vertex_buffers.clear();
	bzero(vertex_offsets.ptr(), sizeof(NS::UInteger) * vertex_offsets.size());
	vertex_offsets.clear();
	encoder_raster = MDRenderPipeline::RasterState();
}

void MDCommandBuffer::RenderState::end_encoding() {
	if (!encoder) {
		return;
	}

	encoder->endEncoding();
	encoder.reset();
}

#pragma mark - ComputeState

void MDCommandBuffer::ComputeState::end_encoding() {
	if (!encoder) {
		return;
	}
	encoder->endEncoding();
	encoder.reset();
}

#pragma mark - Compute

void MDCommandBuffer::compute_begin_pass() {
	ERR_FAIL_COND_MSG(type == MDCommandBufferStateType::Render, "Compute pass cannot begin while a render pass is active.");

	if (type == MDCommandBufferStateType::InlineRender) {
		_end_inline_render();
	} else if (type == MDCommandBufferStateType::Blit) {
		_end_blit();
	}

	type = MDCommandBufferStateType::Compute;
	if (!compute.encoder) {
		compute.encoder = NS::RetainPtr(command_buffer->computeCommandEncoder());
		_fence_wait(compute.encoder.get());
	}
}

void MDCommandBuffer::compute_end_pass() {
	if (type == MDCommandBufferStateType::Compute) {
		_end_compute_dispatch();
	}
}

void MDCommandBuffer::_compute_set_dirty_state() {
	if (compute.dirty.has_flag(ComputeState::DIRTY_PIPELINE)) {
		if (!compute.encoder) {
			compute.encoder = NS::RetainPtr(command_buffer->computeCommandEncoder());
			_fence_wait(compute.encoder.get());
		}
		compute.encoder->setComputePipelineState(compute.pipeline->state.get());
	}

	_compute_bind_uniform_sets();

	if (compute.dirty.has_flag(ComputeState::DIRTY_PUSH) && push_constant_data_len) {
		const MDComputeShader *shader = compute.pipeline->shader;
		if (push_constant_shader == shader) {
			MDRingBuffer::Allocation dst = _scratch.allocate(push_constant_data_len);
			memcpy(dst.ptr, &push_constant_data, push_constant_data_len);
			compute.args->setAddress(dst.gpu_address, shader->push_constants.binding);
		}
	}

	compute.dirty.clear();
}

void MDCommandBuffer::_compute_bind_uniform_sets() {
	DEV_ASSERT(type == MDCommandBufferStateType::Compute);
	if (!compute.dirty.has_flag(ComputeState::DIRTY_UNIFORMS)) {
		return;
	}

	compute.dirty.clear_flag(ComputeState::DIRTY_UNIFORMS);
	uint64_t set_uniforms = compute.uniform_set_mask;
	compute.uniform_set_mask = 0;

	MDComputeShader *shader = compute.pipeline->shader;
	const uint32_t dynamic_offsets = compute.dynamic_offsets;

	while (set_uniforms != 0) {
		// Find the index of the next set bit.
		uint32_t index = (uint32_t)__builtin_ctzll(set_uniforms);
		// Clear the set bit.
		set_uniforms &= (set_uniforms - 1);
		MDUniformSet *set = compute.uniform_sets[index];
		if (set == nullptr || index >= (uint32_t)shader->sets.size()) {
			continue;
		}
		if (shader->uses_argument_buffers) {
			_bind_uniforms_argument_buffers_compute(set, shader, index, dynamic_offsets);
		} else {
			_bind_uniforms_direct(set, shader, compute.residency_set.get(), compute.args.get(), index, dynamic_offsets);
		}
	}

	compute.encoder->setArgumentTable(compute.args.get());
}

void MDCommandBuffer::ComputeState::reset() {
	pipeline = nullptr;
	encoder.reset();
	dirty = DIRTY_NONE;
	uniform_sets.clear();
	dynamic_offsets = 0;
	uniform_set_mask = 0;
}

void MDCommandBuffer::compute_bind_uniform_sets(VectorView<RDD::UniformSetID> p_uniform_sets, RDD::ShaderID p_shader, uint32_t p_first_set_index, uint32_t p_set_count, uint32_t p_dynamic_offsets) {
	DEV_ASSERT(type == MDCommandBufferStateType::Compute);

	if (uint32_t new_size = p_first_set_index + p_set_count; compute.uniform_sets.size() < new_size) {
		uint32_t s = compute.uniform_sets.size();
		compute.uniform_sets.resize(new_size);
		// Set intermediate values to null.
		std::fill(&compute.uniform_sets[s], compute.uniform_sets.end().operator->(), nullptr);
	}

	const MDShader *shader = (const MDShader *)p_shader.id;
	DynamicOffsetLayout layout = shader->dynamic_offset_layout;

	// Clear bits for sets being bound, then OR new values.
	for (uint32_t i = 0; i < p_set_count && compute.dynamic_offsets != 0; i++) {
		uint32_t set_index = p_first_set_index + i;
		uint32_t count = layout.get_count(set_index);
		if (count > 0) {
			uint32_t shift = layout.get_offset_index_shift(set_index);
			uint32_t mask = ((1u << (count * 4u)) - 1u) << shift;
			compute.dynamic_offsets &= ~mask; // Clear this set's bits
		}
	}
	compute.dynamic_offsets |= p_dynamic_offsets;

	for (size_t i = 0; i < p_set_count; ++i) {
		MDUniformSet *set = (MDUniformSet *)(p_uniform_sets[i].id);

		uint32_t index = p_first_set_index + i;
		if (compute.uniform_sets[index] != set || layout.get_count(index) > 0) {
			compute.dirty.set_flag(ComputeState::DIRTY_UNIFORMS);
			compute.uniform_set_mask |= 1ULL << index;
			compute.uniform_sets[index] = set;
		}
	}
}

void MDCommandBuffer::compute_dispatch(uint32_t p_x_groups, uint32_t p_y_groups, uint32_t p_z_groups) {
	DEV_ASSERT(type == MDCommandBufferStateType::Compute);

	_compute_set_dirty_state();

	MTL::Size threadgroups = MTL::Size(p_x_groups, p_y_groups, p_z_groups);

	MTL4::ComputeCommandEncoder *enc = compute.encoder.get();
	enc->dispatchThreadgroups(threadgroups, compute.pipeline->compute_state.local);
}

void MDCommandBuffer::compute_dispatch_indirect(RDD::BufferID p_indirect_buffer, uint64_t p_offset) {
	DEV_ASSERT(type == MDCommandBufferStateType::Compute);

	_compute_set_dirty_state();

	const RenderingDeviceDriverMetal::BufferInfo *indirect_buf = (const RenderingDeviceDriverMetal::BufferInfo *)p_indirect_buffer.id;
	MTL::Buffer *indirectBuffer = indirect_buf->buffer.get();

	MTL4::ComputeCommandEncoder *enc = compute.encoder.get();
	enc->dispatchThreadgroups(indirectBuffer->gpuAddress() + p_offset, compute.pipeline->compute_state.local);
}

void MDCommandBuffer::reset() {
	push_constant_shader = nullptr;
	push_constant_data_len = 0;
	type = MDCommandBufferStateType::None;
}

void MDCommandBuffer::_end_compute_dispatch() {
	DEV_ASSERT(type == MDCommandBufferStateType::Compute);

	_pop_active_encoder_labels();
	_fence_update(compute.encoder.get());
	compute.end_encoding();
	compute.reset();
	reset();
}

void MDCommandBuffer::_set_inline_render_encoder(MTL4::RenderCommandEncoder *p_encoder) {
	DEV_ASSERT(p_encoder != nullptr);
	DEV_ASSERT(!render.encoder);
	DEV_ASSERT(!inline_render.encoder);

	type = MDCommandBufferStateType::InlineRender;
	inline_render.encoder = NS::RetainPtr(p_encoder);
}

void MDCommandBuffer::_end_inline_render() {
	DEV_ASSERT(type == MDCommandBufferStateType::InlineRender);

	_pop_active_encoder_labels();
	_fence_update(inline_render.encoder.get());
	inline_render.encoder->endEncoding();
	inline_render.reset();
	reset();
}

void MDCommandBuffer::_end_blit() {
	DEV_ASSERT(type == MDCommandBufferStateType::Blit);

	_pop_active_encoder_labels();
	_fence_update(blit.encoder.get());
	blit.encoder->endEncoding();
	blit.reset();
	reset();
}

#pragma mark - Command Pool

MDCommandBuffer *MD4CommandPool::new_command_buffer() {
	MDCommandBuffer *obj = memnew(MDCommandBuffer(_driver->get_device()->newCommandAllocator(), _driver));
	_command_buffers.push_back(obj);
	return obj;
}

MD4CommandPool::MD4CommandPool(RenderingDeviceDriverMetal *p_driver) :
		_driver(p_driver) {
}

MD4CommandPool::~MD4CommandPool() {
	for (MDCommandBuffer *cb : _command_buffers) {
		memdelete(cb);
	}
}

void MDCommandBuffer::_bind_uniforms_argument_buffers(MDUniformSet *p_set, MDShader *p_shader, uint32_t p_set_index, uint32_t p_dynamic_offsets) {
	DEV_ASSERT(p_shader->uses_argument_buffers);
	DEV_ASSERT(render.encoder);

	const UniformSet &shader_set = p_shader->sets[p_set_index];

	// Check if this set has dynamic uniforms.
	if (!shader_set.dynamic_uniforms.is_empty()) {
		// Allocate from the scratch buffer.
		uint32_t buffer_size = p_set->arg_buffer_data.size();
		MDRingBuffer::Allocation alloc = _scratch.allocate(buffer_size);

		// Copy the base argument buffer data.
		memcpy(alloc.ptr, p_set->arg_buffer_data.ptr(), buffer_size);

		// Update dynamic buffer GPU addresses.
		uint64_t *ptr = (uint64_t *)alloc.ptr;
		DynamicOffsetLayout layout = p_shader->dynamic_offset_layout;
		uint32_t dynamic_index = 0;

		for (uint32_t i : shader_set.dynamic_uniforms) {
			const RDD::BoundUniform &uniform = p_set->uniforms[i];
			const UniformInfo &ui = shader_set.uniforms[i];
			const UniformInfo::Indexes &idx = ui.arg_buffer;

			uint32_t shift = layout.get_offset_index_shift(p_set_index, dynamic_index);
			dynamic_index++;
			uint32_t frame_idx = (p_dynamic_offsets >> shift) & 0xf;

			const MetalBufferDynamicInfo *buf_info = (const MetalBufferDynamicInfo *)uniform.ids[0].id;
			uint64_t gpu_address = buf_info->buffer.get()->gpuAddress() + frame_idx * buf_info->size_bytes;
			*(uint64_t *)(ptr + idx.buffer) = gpu_address;
		}

		render.args->setAddress(alloc.gpu_address, p_set_index);
	} else {
		render.args->setAddress(p_set->arg_buffer.buffer->gpuAddress(), p_set_index);
	}
}

void MDCommandBuffer::_bind_uniforms_direct(MDUniformSet *p_set, MDShader *p_shader, MTL::ResidencySet *p_rs, MTL4::ArgumentTable *p_args, uint32_t p_set_index, uint32_t p_dynamic_offsets) {
	DEV_ASSERT(!p_shader->uses_argument_buffers);

	const UniformSet &set = p_shader->sets[p_set_index];
	DynamicOffsetLayout layout = p_shader->dynamic_offset_layout;
	uint32_t dynamic_index = 0;

	for (uint32_t i = 0; i < MIN(p_set->uniforms.size(), set.uniforms.size()); i++) {
		const RDD::BoundUniform &uniform = p_set->uniforms[i];
		const UniformInfo &ui = set.uniforms[i];
		const UniformInfo::Indexes &indexes = ui.slot;

		uint32_t frame_idx;
		if (uniform.is_dynamic()) {
			uint32_t shift = layout.get_offset_index_shift(p_set_index, dynamic_index);
			dynamic_index++;
			frame_idx = (p_dynamic_offsets >> shift) & 0xf;
		} else {
			frame_idx = 0;
		}

		switch (uniform.type) {
			case RDD::UNIFORM_TYPE_SAMPLER: {
				size_t count = uniform.ids.size();
				for (size_t j = 0; j < count; j += 1) {
					MTL::SamplerState *obj = rid::get<MTL::SamplerState>(uniform.ids[j].id);
					p_args->setSamplerState(obj->gpuResourceID(), indexes.sampler + j);
				}
			} break;
			case RDD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE: {
				size_t count = uniform.ids.size() / 2;
				for (uint32_t j = 0; j < count; j += 1) {
					MTL::SamplerState *sampler = rid::get<MTL::SamplerState>(uniform.ids[j * 2 + 0]);
					MTL::Texture *texture = rid::get<RDM::TextureInfo>(uniform.ids[j * 2 + 1])->texture.get();
					p_args->setSamplerState(sampler->gpuResourceID(), indexes.sampler + j);
					p_args->setTexture(texture->gpuResourceID(), indexes.texture + j);
				}
			} break;
			case RDD::UNIFORM_TYPE_TEXTURE: {
				size_t count = uniform.ids.size();
				for (size_t j = 0; j < count; j += 1) {
					MTL::Texture *obj = rid::get<RDM::TextureInfo>(uniform.ids[j])->texture.get();
					p_args->setTexture(obj->gpuResourceID(), indexes.texture + j);
				}
			} break;
			case RDD::UNIFORM_TYPE_IMAGE: {
				size_t count = uniform.ids.size();
				for (size_t j = 0; j < count; j += 1) {
					MTL::Texture *obj = rid::get<RDM::TextureInfo>(uniform.ids[j])->texture.get();
					p_args->setTexture(obj->gpuResourceID(), indexes.texture + j);
				}

				if (indexes.buffer != UINT32_MAX) {
					// Emulated atomic image access.
					for (size_t j = 0; j < count; j += 1) {
						MTL::Texture *obj = rid::get<RDM::TextureInfo>(uniform.ids[j])->texture.get();
						MTL::Texture *tex = obj->parentTexture() ? obj->parentTexture() : obj;
						MTL::Buffer *buf = tex->buffer();
						p_args->setAddress(buf->gpuAddress() + tex->bufferOffset(), indexes.buffer + j);
					}
				}
			} break;
			case RDD::UNIFORM_TYPE_TEXTURE_BUFFER: {
				ERR_PRINT("not implemented: UNIFORM_TYPE_TEXTURE_BUFFER");
			} break;
			case RDD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE_BUFFER: {
				ERR_PRINT("not implemented: UNIFORM_TYPE_SAMPLER_WITH_TEXTURE_BUFFER");
			} break;
			case RDD::UNIFORM_TYPE_IMAGE_BUFFER: {
				CRASH_NOW_MSG("not implemented: UNIFORM_TYPE_IMAGE_BUFFER");
			} break;
			case RDD::UNIFORM_TYPE_UNIFORM_BUFFER:
			case RDD::UNIFORM_TYPE_STORAGE_BUFFER: {
				const RDM::BufferInfo *buf_info = (const RDM::BufferInfo *)uniform.ids[0].id;
				p_args->setAddress(buf_info->buffer.get()->gpuAddress(), indexes.buffer);
			} break;
			case RDD::UNIFORM_TYPE_UNIFORM_BUFFER_DYNAMIC:
			case RDD::UNIFORM_TYPE_STORAGE_BUFFER_DYNAMIC: {
				const MetalBufferDynamicInfo *buf_info = (const MetalBufferDynamicInfo *)uniform.ids[0].id;
				p_args->setAddress(buf_info->buffer.get()->gpuAddress() + frame_idx * buf_info->size_bytes, indexes.buffer);
			} break;
			case RDD::UNIFORM_TYPE_INPUT_ATTACHMENT: {
				size_t count = uniform.ids.size();

				for (size_t j = 0; j < count; j += 1) {
					MTL::Texture *obj = rid::get<RDM::TextureInfo>(uniform.ids[j])->texture.get();
					p_args->setTexture(obj->gpuResourceID(), indexes.texture + j);
				}
			} break;
			default: {
				DEV_ASSERT(false);
			}
		}
	}
}

void MDCommandBuffer::_bind_uniforms_argument_buffers_compute(MDUniformSet *p_set, MDShader *p_shader, uint32_t p_set_index, uint32_t p_dynamic_offsets) {
	DEV_ASSERT(p_shader->uses_argument_buffers);
	DEV_ASSERT(compute.encoder);

	const UniformSet &shader_set = p_shader->sets[p_set_index];

	// Check if this set has dynamic uniforms.
	if (!shader_set.dynamic_uniforms.is_empty()) {
		// Allocate from the scratch buffer.
		uint32_t buffer_size = p_set->arg_buffer_data.size();
		MDRingBuffer::Allocation alloc = _scratch.allocate(buffer_size);

		// Copy the base argument buffer data.
		memcpy(alloc.ptr, p_set->arg_buffer_data.ptr(), buffer_size);

		// Update dynamic buffer GPU addresses.
		uint64_t *ptr = (uint64_t *)alloc.ptr;
		DynamicOffsetLayout layout = p_shader->dynamic_offset_layout;
		uint32_t dynamic_index = 0;

		for (uint32_t i : shader_set.dynamic_uniforms) {
			const RDD::BoundUniform &uniform = p_set->uniforms[i];
			const UniformInfo &ui = shader_set.uniforms[i];
			const UniformInfo::Indexes &idx = ui.arg_buffer;

			uint32_t shift = layout.get_offset_index_shift(p_set_index, dynamic_index);
			dynamic_index++;
			uint32_t frame_idx = (p_dynamic_offsets >> shift) & 0xf;

			const MetalBufferDynamicInfo *buf_info = (const MetalBufferDynamicInfo *)uniform.ids[0].id;
			uint64_t gpu_address = buf_info->buffer.get()->gpuAddress() + frame_idx * buf_info->size_bytes;
			*(uint64_t *)(ptr + idx.buffer) = gpu_address;
		}

		compute.args->setAddress(alloc.gpu_address, p_set_index);
	} else {
		compute.args->setAddress(p_set->arg_buffer.buffer->gpuAddress(), p_set_index);
	}
}

#pragma mark - Timestamp

void MDCommandBuffer::timestamp_write(QueryPool *p_pool, uint32_t p_index) {
	end();
	command_buffer->writeTimestampIntoHeap(p_pool->get_counter_heap(), p_index);
}

#pragma mark - QueryPool

QueryPool::QueryPool(NS::SharedPtr<MTL4::CounterHeap> p_heap, uint32_t p_count, uint64_t p_frequency) :
		counter_heap(p_heap), timestamp_frequency(p_frequency), timestamp_to_nano(1000000000.0 / (double)p_frequency), count(p_count) {
}

void QueryPool::invalidate(uint32_t p_count) {
	counter_heap->invalidateCounterRange(NS::Range::Make(0, p_count));
}

void QueryPool::get_results(uint32_t p_count, uint64_t *r_results) {
	NS::Data *data = counter_heap->resolveCounterRange(NS::Range::Make(0, p_count));
	const uint64_t *src = (const uint64_t *)data->bytes();
	for (uint32_t i = 0; i < p_count; i++) {
		r_results[i] = result_to_time(src[i]);
	}
}

static const char *SHADER_STAGE_NAMES[] = {
	[RDD::SHADER_STAGE_VERTEX] = "vert",
	[RDD::SHADER_STAGE_FRAGMENT] = "frag",
	[RDD::SHADER_STAGE_TESSELATION_CONTROL] = "tess_ctrl",
	[RDD::SHADER_STAGE_TESSELATION_EVALUATION] = "tess_eval",
	[RDD::SHADER_STAGE_COMPUTE] = "comp",
};

GODOT_CLANG_WARNING_POP

#endif // METAL4_ENABLED
