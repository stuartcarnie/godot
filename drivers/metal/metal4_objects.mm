/**************************************************************************/
/*  metal4_objects.mm                                                     */
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

#import "metal4_objects.h"

#import "metal_utils.h"
#import "pixel_formats.h"
#import "rendering_device_driver_metal4.h"
#import "rendering_shader_container_metal.h"

#import <os/signpost.h>
#import <algorithm>

// We have to undefine these macros because they are defined in NSObjCRuntime.h.
#undef MIN
#undef MAX

namespace MTL4 {

GODOT_CLANG_WARNING_PUSH_AND_IGNORE("-Wunguarded-availability-new")

MDCommandBuffer::MDCommandBuffer(id<MTL4CommandAllocator> p_allocator, RenderingDeviceDriverMetal *p_device_driver) :
		allocator(p_allocator), command_buffer([p_device_driver->get_device() newCommandBuffer]), _scratch(p_device_driver->get_device(), DEFAULT_SCRATCH_SIZE) {
	device_driver = p_device_driver;
	id<MTLDevice> device = device_driver->get_device();

	MTLResidencySetDescriptor *rs_desc = [MTLResidencySetDescriptor new];
	[rs_desc setInitialCapacity:10];
	rs_desc.label = @"Command Residency Set";
	NSError *error;
	_frame_state.rs = [device newResidencySetWithDescriptor:rs_desc error:&error];
	CRASH_COND_MSG(error != nil, vformat("Failed to create residency set: %s", String(error.localizedDescription.UTF8String)));

	MTL4ArgumentTableDescriptor *desc = [MTL4ArgumentTableDescriptor new];
	desc.maxBufferBindCount = 31;
	desc.maxTextureBindCount = 64;
	desc.maxSamplerStateBindCount = 16;

	render.args = [device newArgumentTableWithDescriptor:desc error:nil];
	compute.args = [device newArgumentTableWithDescriptor:desc error:nil];
	_args_clear = [device newArgumentTableWithDescriptor:desc error:nil];
}

MDCommandBuffer::~MDCommandBuffer() {
}

void MDCommandBuffer::begin_label(const char *p_label_name, const Color &p_color) {
	NSString *s = [[NSString alloc] initWithBytesNoCopy:(void *)p_label_name length:strlen(p_label_name) encoding:NSUTF8StringEncoding freeWhenDone:NO];
	[command_buffer pushDebugGroup:s];
}

void MDCommandBuffer::end_label() {
	[command_buffer popDebugGroup];
}

void MDCommandBuffer::begin() {
	DEV_ASSERT(!state_begin);
	state_begin = true;
	bzero(pending_after_stages, sizeof(pending_after_stages));
	bzero(pending_before_queue_stages, sizeof(pending_before_queue_stages));

	[allocator reset];
	_scratch.reset();
	release_resources();

	[command_buffer beginCommandBufferWithAllocator:allocator];
	[command_buffer useResidencySet:_frame_state.rs];
}

void MDCommandBuffer::end() {
	switch (type) {
		case MDCommandBufferStateType::None:
		case MDCommandBufferStateType::Blit:
			return;
		case MDCommandBufferStateType::Render:
			return render_end_pass();
		case MDCommandBufferStateType::Compute:
			return _end_compute_dispatch();
	}
}

void MDCommandBuffer::commit() {
	end();

	render.residency_set = nil;
	compute.residency_set = nil;

	if (_scratch.is_changed()) {
		for (id<MTLBuffer> buf : _scratch.get_buffers()) {
			[_frame_state.rs addAllocation:buf];
		}
		_scratch.clear_changed();
		[_frame_state.rs commit];
	}

	[command_buffer endCommandBuffer];
	state_begin = false;
}

void MDCommandBuffer::_encode_barrier(id<MTL4CommandEncoder> p_enc) {
	DEV_ASSERT(p_enc);

	static const MTLStages empty_stages[STAGE_MAX] = { 0, 0 };
	if (memcmp(&pending_before_queue_stages, empty_stages, sizeof(pending_before_queue_stages)) == 0) {
		return;
	}

	int stage = STAGE_MAX;
	if ([p_enc conformsToProtocol:@protocol(MTL4RenderCommandEncoder)] && pending_after_stages[STAGE_RENDER] != 0) {
		stage = STAGE_RENDER;
	} else if ([p_enc conformsToProtocol:@protocol(MTL4ComputeCommandEncoder)] && pending_after_stages[STAGE_COMPUTE] != 0) {
		stage = STAGE_COMPUTE;
	}

	if (stage == STAGE_MAX) {
		return;
	}

	[p_enc barrierAfterQueueStages:pending_after_stages[stage] beforeStages:pending_before_queue_stages[stage] visibilityOptions:MTL4VisibilityOptionDevice];
	pending_before_queue_stages[stage] = 0;
	pending_after_stages[stage] = 0;
}

void MDCommandBuffer::pipeline_barrier(BitField<RDD::PipelineStageBits> p_src_stages,
		BitField<RDD::PipelineStageBits> p_dst_stages,
		VectorView<RDD::MemoryAccessBarrier> p_memory_barriers,
		VectorView<RDD::BufferBarrier> p_buffer_barriers,
		VectorView<RDD::TextureBarrier> p_texture_barriers) {
	MTLStages after_stages = convert_src_pipeline_stages_to_metal(p_src_stages);
	if (after_stages == 0) {
		return;
	}

	MTLStages before_stages = convert_dst_pipeline_stages_to_metal(p_dst_stages);
	if (before_stages == 0) {
		return;
	}

	// Encode intra-pass barrier if an encoder is active for matching stages.
	if (render.encoder != nil) {
		MTLStages render_after = after_stages & (MTLStageVertex | MTLStageFragment);
		MTLStages render_before = before_stages & (MTLStageVertex | MTLStageFragment);
		if (render_after != 0 && render_before != 0) {
			[render.encoder barrierAfterEncoderStages:render_after beforeEncoderStages:render_before visibilityOptions:MTL4VisibilityOptionDevice];
		}
	} else if (compute.encoder != nil) {
		MTLStages compute_after = after_stages & (MTLStageDispatch | MTLStageBlit);
		MTLStages compute_before = before_stages & (MTLStageDispatch | MTLStageBlit);
		if (compute_after != 0 && compute_before != 0) {
			[compute.encoder barrierAfterEncoderStages:compute_after beforeEncoderStages:compute_before visibilityOptions:MTL4VisibilityOptionDevice];
		}
	}

	// Also cache for inter-pass barriers.
	if (after_stages & (MTLStageVertex | MTLStageFragment)) {
		pending_after_stages[STAGE_RENDER] |= after_stages;
		pending_before_queue_stages[STAGE_RENDER] |= before_stages;
	}

	if (after_stages & (MTLStageDispatch | MTLStageBlit)) {
		pending_after_stages[STAGE_COMPUTE] |= after_stages;
		pending_before_queue_stages[STAGE_COMPUTE] |= before_stages;
	}
}

void MDCommandBuffer::bind_pipeline(RDD::PipelineID p_pipeline) {
	MDPipeline *p = (MDPipeline *)(p_pipeline.id);

	// End current encoder if it is a compute encoder or blit encoder,
	// as they do not have a defined end boundary in the RDD like render.
	if (type == MDCommandBufferStateType::Compute) {
		_end_compute_dispatch();
	}

	if (p->type == MDPipelineType::Render) {
		DEV_ASSERT(type == MDCommandBufferStateType::Render);
		MDRenderPipeline *rp = (MDRenderPipeline *)p;

		if (render.encoder == nil) {
			// This error would happen if the render pass failed.
			ERR_FAIL_NULL_MSG(render.desc, "Render pass descriptor is null.");

			// This condition occurs when there are no attachments when calling render_next_subpass()
			// and is due to the SUPPORTS_FRAGMENT_SHADER_WITH_ONLY_SIDE_EFFECTS flag.
			render.desc.defaultRasterSampleCount = static_cast<NSUInteger>(rp->sample_count);

// NOTE(sgc): This is to test rdar://FB13605547 and will be deleted once fix is confirmed.
#if 0
			if (render.pipeline->sample_count == 4) {
				static id<MTLTexture> tex = nil;
				static id<MTLTexture> res_tex = nil;
				static dispatch_once_t onceToken;
				dispatch_once(&onceToken, ^{
					Size2i sz = render.frameBuffer->size;
					MTLTextureDescriptor *td = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm width:sz.width height:sz.height mipmapped:NO];
					td.textureType = MTLTextureType2DMultisample;
					td.storageMode = MTLStorageModeMemoryless;
					td.usage = MTLTextureUsageRenderTarget;
					td.sampleCount = render.pipeline->sample_count;
					tex = [device_driver->get_device() newTextureWithDescriptor:td];

					td.textureType = MTLTextureType2D;
					td.storageMode = MTLStorageModePrivate;
					td.usage = MTLTextureUsageShaderWrite;
					td.sampleCount = 1;
					res_tex = [device_driver->get_device() newTextureWithDescriptor:td];
				});
				render.desc.colorAttachments[0].texture = tex;
				render.desc.colorAttachments[0].loadAction = MTLLoadActionClear;
				render.desc.colorAttachments[0].storeAction = MTLStoreActionMultisampleResolve;

				render.desc.colorAttachments[0].resolveTexture = res_tex;
			}
#endif
			render.encoder = [command_buffer renderCommandEncoderWithDescriptor:render.desc];
			_encode_barrier(render.encoder);
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
		DEV_ASSERT(type == MDCommandBufferStateType::None);
		type = MDCommandBufferStateType::Compute;

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

id<MTL4ComputeCommandEncoder> MDCommandBuffer::_ensure_blit_encoder() {
	switch (type) {
		case MDCommandBufferStateType::None:
		case MDCommandBufferStateType::Blit:
			break;
		case MDCommandBufferStateType::Render:
			render_end_pass();
			break;
		case MDCommandBufferStateType::Compute:
			return compute.encoder;
	}

	type = MDCommandBufferStateType::Compute;
	compute.encoder = [command_buffer computeCommandEncoder];
	_encode_barrier(compute.encoder);
	return compute.encoder;
}

void MDCommandBuffer::resolve_texture(RDD::TextureID p_src_texture, RDD::TextureLayout p_src_texture_layout, uint32_t p_src_layer, uint32_t p_src_mipmap, RDD::TextureID p_dst_texture, RDD::TextureLayout p_dst_texture_layout, uint32_t p_dst_layer, uint32_t p_dst_mipmap) {
	id<MTLTexture> src_tex = rid::get(p_src_texture);
	id<MTLTexture> dst_tex = rid::get(p_dst_texture);

	MTL4RenderPassDescriptor *mtlRPD = [MTL4RenderPassDescriptor new];
	MTLRenderPassColorAttachmentDescriptor *mtlColorAttDesc = mtlRPD.colorAttachments[0];
	mtlColorAttDesc.loadAction = MTLLoadActionLoad;
	mtlColorAttDesc.storeAction = MTLStoreActionMultisampleResolve;

	mtlColorAttDesc.texture = src_tex;
	mtlColorAttDesc.resolveTexture = dst_tex;
	mtlColorAttDesc.level = p_src_mipmap;
	mtlColorAttDesc.slice = p_src_layer;
	mtlColorAttDesc.resolveLevel = p_dst_mipmap;
	mtlColorAttDesc.resolveSlice = p_dst_layer;

	id<MTL4RenderCommandEncoder> enc = get_new_render_encoder_with_descriptor(mtlRPD);
	enc.label = @"Resolve Image";
	[enc endEncoding];
}

void MDCommandBuffer::clear_color_texture(RDD::TextureID p_texture, RDD::TextureLayout p_texture_layout, const Color &p_color, const RDD::TextureSubresourceRange &p_subresources) {
	id<MTLTexture> src_tex = rid::get(p_texture);

	if (src_tex.parentTexture) {
		// Clear via the parent texture rather than the view.
		src_tex = src_tex.parentTexture;
	}

	PixelFormats &pf = device_driver->get_pixel_formats();

	if (pf.isDepthFormat(src_tex.pixelFormat) || pf.isStencilFormat(src_tex.pixelFormat)) {
		ERR_FAIL_MSG("invalid: depth or stencil texture format");
	}

	MTL4RenderPassDescriptor *desc = [MTL4RenderPassDescriptor new];

	if (p_subresources.aspect.has_flag(RDD::TEXTURE_ASPECT_COLOR_BIT)) {
		MTLRenderPassColorAttachmentDescriptor *caDesc = desc.colorAttachments[0];
		caDesc.texture = src_tex;
		caDesc.loadAction = MTLLoadActionClear;
		caDesc.storeAction = MTLStoreActionStore;
		caDesc.clearColor = MTLClearColorMake(p_color.r, p_color.g, p_color.b, p_color.a);

		// Extract the mipmap levels that are to be updated.
		uint32_t mipLvlStart = p_subresources.base_mipmap;
		uint32_t mipLvlCnt = p_subresources.mipmap_count;
		uint32_t mipLvlEnd = mipLvlStart + mipLvlCnt;

		uint32_t levelCount = src_tex.mipmapLevelCount;

		// Extract the cube or array layers (slices) that are to be updated.
		bool is3D = src_tex.textureType == MTLTextureType3D;
		uint32_t layerStart = is3D ? 0 : p_subresources.base_layer;
		uint32_t layerCnt = p_subresources.layer_count;
		uint32_t layerEnd = layerStart + layerCnt;

		MetalFeatures const &features = device_driver->get_device_properties().features;

		// Iterate across mipmap levels and layers, and perform an empty render to clear each.
		for (uint32_t mipLvl = mipLvlStart; mipLvl < mipLvlEnd; mipLvl++) {
			ERR_FAIL_INDEX_MSG(mipLvl, levelCount, "mip level out of range");

			caDesc.level = mipLvl;

			// If a 3D image, we need to get the depth for each level.
			if (is3D) {
				layerCnt = mipmapLevelSizeFromTexture(src_tex, mipLvl).depth;
				layerEnd = layerStart + layerCnt;
			}

			if ((features.layeredRendering && src_tex.sampleCount == 1) || features.multisampleLayeredRendering) {
				// We can clear all layers at once.
				if (is3D) {
					caDesc.depthPlane = layerStart;
				} else {
					caDesc.slice = layerStart;
				}
				desc.renderTargetArrayLength = layerCnt;
				id<MTL4RenderCommandEncoder> enc = get_new_render_encoder_with_descriptor(desc);
				enc.label = @"Clear Image";
				[enc endEncoding];
			} else {
				for (uint32_t layer = layerStart; layer < layerEnd; layer++) {
					if (is3D) {
						caDesc.depthPlane = layer;
					} else {
						caDesc.slice = layer;
					}
					id<MTL4RenderCommandEncoder> enc = get_new_render_encoder_with_descriptor(desc);
					enc.label = @"Clear Image";
					[enc endEncoding];
				}
			}
		}
	}
}

void MDCommandBuffer::clear_buffer(RDD::BufferID p_buffer, uint64_t p_offset, uint64_t p_size) {
	id<MTL4ComputeCommandEncoder> blit_enc = _ensure_blit_encoder();
	const RDM::BufferInfo *buffer = (const RDM::BufferInfo *)p_buffer.id;

	[blit_enc fillBuffer:buffer->metal_buffer
				   range:NSMakeRange(p_offset, p_size)
				   value:0];
}

void MDCommandBuffer::copy_buffer(RDD::BufferID p_src_buffer, RDD::BufferID p_dst_buffer, VectorView<RDD::BufferCopyRegion> p_regions) {
	const RDM::BufferInfo *src = (const RDM::BufferInfo *)p_src_buffer.id;
	const RDM::BufferInfo *dst = (const RDM::BufferInfo *)p_dst_buffer.id;

	id<MTL4ComputeCommandEncoder> enc = _ensure_blit_encoder();

	for (uint32_t i = 0; i < p_regions.size(); i++) {
		RDD::BufferCopyRegion region = p_regions[i];
		[enc copyFromBuffer:src->metal_buffer
					 sourceOffset:region.src_offset
						 toBuffer:dst->metal_buffer
				destinationOffset:region.dst_offset
							 size:region.size];
	}
}

void MDCommandBuffer::copy_texture(RDD::TextureID p_src_texture, RDD::TextureID p_dst_texture, VectorView<RDD::TextureCopyRegion> p_regions) {
	id<MTLTexture> src = rid::get(p_src_texture);
	id<MTLTexture> dst = rid::get(p_dst_texture);

	id<MTL4ComputeCommandEncoder> enc = _ensure_blit_encoder();
	PixelFormats &pf = device_driver->get_pixel_formats();

	MTLPixelFormat src_fmt = src.pixelFormat;
	bool src_is_compressed = pf.getFormatType(src_fmt) == MTLFormatType::Compressed;
	MTLPixelFormat dst_fmt = dst.pixelFormat;
	bool dst_is_compressed = pf.getFormatType(dst_fmt) == MTLFormatType::Compressed;

	// Validate copy.
	if (src.sampleCount != dst.sampleCount || pf.getBytesPerBlock(src_fmt) != pf.getBytesPerBlock(dst_fmt)) {
		ERR_FAIL_MSG("Cannot copy between incompatible pixel formats, such as formats of different pixel sizes, or between images with different sample counts.");
	}

	// If source and destination have different formats and at least one is compressed, a temporary buffer is required.
	bool need_tmp_buffer = (src_fmt != dst_fmt) && (src_is_compressed || dst_is_compressed);
	if (need_tmp_buffer) {
		ERR_FAIL_MSG("not implemented: copy with intermediate buffer");
	}

	if (src_fmt != dst_fmt) {
		// Map the source pixel format to the dst through a texture view on the source texture.
		src = [src newTextureViewWithPixelFormat:dst_fmt];
	}

	for (uint32_t i = 0; i < p_regions.size(); i++) {
		RDD::TextureCopyRegion region = p_regions[i];

		MTLSize extent = MTLSizeFromVector3i(region.size);

		// If copies can be performed using direct texture-texture copying, do so.
		uint32_t src_level = region.src_subresources.mipmap;
		uint32_t src_base_layer = region.src_subresources.base_layer;
		MTLSize src_extent = mipmapLevelSizeFromTexture(src, src_level);
		uint32_t dst_level = region.dst_subresources.mipmap;
		uint32_t dst_base_layer = region.dst_subresources.base_layer;
		MTLSize dst_extent = mipmapLevelSizeFromTexture(dst, dst_level);

		// All layers may be copied at once, if the extent completely covers both images.
		if (src_extent == extent && dst_extent == extent) {
			[enc copyFromTexture:src
						 sourceSlice:src_base_layer
						 sourceLevel:src_level
						   toTexture:dst
					destinationSlice:dst_base_layer
					destinationLevel:dst_level
						  sliceCount:region.src_subresources.layer_count
						  levelCount:1];
		} else {
			MTLOrigin src_origin = MTLOriginFromVector3i(region.src_offset);
			MTLSize src_size = clampMTLSize(extent, src_origin, src_extent);
			uint32_t layer_count = 0;
			if ((src.textureType == MTLTextureType3D) != (dst.textureType == MTLTextureType3D)) {
				// In the case, the number of layers to copy is in extent.depth. Use that value,
				// then clamp the depth, so we don't try to copy more than Metal will allow.
				layer_count = extent.depth;
				src_size.depth = 1;
			} else {
				layer_count = region.src_subresources.layer_count;
			}
			MTLOrigin dst_origin = MTLOriginFromVector3i(region.dst_offset);

			for (uint32_t layer = 0; layer < layer_count; layer++) {
				// We can copy between a 3D and a 2D image easily. Just copy between
				// one slice of the 2D image and one plane of the 3D image at a time.
				if ((src.textureType == MTLTextureType3D) == (dst.textureType == MTLTextureType3D)) {
					[enc copyFromTexture:src
								  sourceSlice:src_base_layer + layer
								  sourceLevel:src_level
								 sourceOrigin:src_origin
								   sourceSize:src_size
									toTexture:dst
							 destinationSlice:dst_base_layer + layer
							 destinationLevel:dst_level
							destinationOrigin:dst_origin];
				} else if (src.textureType == MTLTextureType3D) {
					[enc copyFromTexture:src
								  sourceSlice:src_base_layer
								  sourceLevel:src_level
								 sourceOrigin:MTLOriginMake(src_origin.x, src_origin.y, src_origin.z + layer)
								   sourceSize:src_size
									toTexture:dst
							 destinationSlice:dst_base_layer + layer
							 destinationLevel:dst_level
							destinationOrigin:dst_origin];
				} else {
					DEV_ASSERT(dst.textureType == MTLTextureType3D);
					[enc copyFromTexture:src
								  sourceSlice:src_base_layer + layer
								  sourceLevel:src_level
								 sourceOrigin:src_origin
								   sourceSize:src_size
									toTexture:dst
							 destinationSlice:dst_base_layer
							 destinationLevel:dst_level
							destinationOrigin:MTLOriginMake(dst_origin.x, dst_origin.y, dst_origin.z + layer)];
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
	id<MTLTexture> texture = rid::get(p_texture);

	id<MTL4ComputeCommandEncoder> enc = _ensure_blit_encoder();

	PixelFormats &pf = device_driver->get_pixel_formats();
	MTLPixelFormat mtlPixFmt = texture.pixelFormat;

	MTLBlitOption options = MTLBlitOptionNone;
	if (pf.isPVRTCFormat(mtlPixFmt)) {
		options |= MTLBlitOptionRowLinearPVRTC;
	}

	for (uint32_t i = 0; i < p_regions.size(); i++) {
		RDD::BufferTextureCopyRegion region = p_regions[i];

		uint32_t mip_level = region.texture_subresource.mipmap;
		MTLOrigin txt_origin = MTLOriginMake(region.texture_offset.x, region.texture_offset.y, region.texture_offset.z);
		MTLSize src_extent = mipmapLevelSizeFromTexture(texture, mip_level);
		MTLSize txt_size = clampMTLSize(MTLSizeMake(region.texture_region_size.x, region.texture_region_size.y, region.texture_region_size.z),
				txt_origin,
				src_extent);

		uint32_t buffImgWd = region.texture_region_size.x;
		uint32_t buffImgHt = region.texture_region_size.y;

		NSUInteger bytesPerRow = pf.getBytesPerRow(mtlPixFmt, buffImgWd);
		NSUInteger bytesPerImg = pf.getBytesPerLayer(mtlPixFmt, bytesPerRow, buffImgHt);

		MTLBlitOption blit_options = options;

		if (pf.isDepthFormat(mtlPixFmt) && pf.isStencilFormat(mtlPixFmt)) {
			// Don't reduce depths of 32-bit depth/stencil formats.
			if (region.texture_subresource.aspect == RDD::TEXTURE_ASPECT_DEPTH) {
				if (pf.getBytesPerTexel(mtlPixFmt) != 4) {
					bytesPerRow -= buffImgWd;
					bytesPerImg -= buffImgWd * buffImgHt;
				}
				blit_options |= MTLBlitOptionDepthFromDepthStencil;
			} else if (region.texture_subresource.aspect == RDD::TEXTURE_ASPECT_STENCIL) {
				// The stencil component is always 1 byte per pixel.
				bytesPerRow = buffImgWd;
				bytesPerImg = buffImgWd * buffImgHt;
				blit_options |= MTLBlitOptionStencilFromDepthStencil;
			}
		}

		if (!isArrayTexture(texture.textureType)) {
			bytesPerImg = 0;
		}

		if (p_source == CopySource::Buffer) {
			[enc copyFromBuffer:buffer->metal_buffer
						   sourceOffset:region.buffer_offset
					  sourceBytesPerRow:bytesPerRow
					sourceBytesPerImage:bytesPerImg
							 sourceSize:txt_size
							  toTexture:texture
					   destinationSlice:region.texture_subresource.layer
					   destinationLevel:mip_level
					  destinationOrigin:txt_origin
								options:blit_options];
		} else {
			[enc copyFromTexture:texture
								 sourceSlice:region.texture_subresource.layer
								 sourceLevel:mip_level
								sourceOrigin:txt_origin
								  sourceSize:txt_size
									toBuffer:buffer->metal_buffer
						   destinationOffset:region.buffer_offset
					  destinationBytesPerRow:bytesPerRow
					destinationBytesPerImage:bytesPerImg
									 options:blit_options];
		}
	}
}

id<MTL4RenderCommandEncoder> MDCommandBuffer::get_new_render_encoder_with_descriptor(MTL4RenderPassDescriptor *p_desc) {
	switch (type) {
		case MDCommandBufferStateType::None:
		case MDCommandBufferStateType::Blit:
			break;
		case MDCommandBufferStateType::Render:
			render_end_pass();
			break;
		case MDCommandBufferStateType::Compute:
			_end_compute_dispatch();
			break;
	}

	id<MTL4RenderCommandEncoder> enc = [command_buffer renderCommandEncoderWithDescriptor:p_desc];
	_encode_barrier(enc);
	return enc;
}

#pragma mark - Render Commands

void MDCommandBuffer::render_bind_uniform_sets(VectorView<RDD::UniformSetID> p_uniform_sets, RDD::ShaderID p_shader, uint32_t p_first_set_index, uint32_t p_set_count, uint32_t p_dynamic_offsets) {
	DEV_ASSERT(type == MDCommandBufferStateType::Render);

	render.dynamic_offsets |= p_dynamic_offsets;

	if (uint32_t new_size = p_first_set_index + p_set_count; render.uniform_sets.size() < new_size) {
		uint32_t s = render.uniform_sets.size();
		render.uniform_sets.resize(new_size);
		// Set intermediate values to null.
		std::fill(&render.uniform_sets[s], render.uniform_sets.end().operator->(), nullptr);
	}

	const MDShader *shader = (const MDShader *)p_shader.id;
	DynamicOffsetLayout layout = shader->dynamic_offset_layout;

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
		RDD::AttachmentClear const &attClear = p_attachment_clears[i];
		uint32_t attachment_index;
		if (attClear.aspect.has_flag(RDD::TEXTURE_ASPECT_COLOR_BIT)) {
			attachment_index = attClear.color_attachment;
		} else {
			attachment_index = subpass.depth_stencil_reference.attachment;
		}

		MDAttachment const &mda = render.pass->attachments[attachment_index];
		if (attClear.aspect.has_flag(RDD::TEXTURE_ASPECT_COLOR_BIT)) {
			key.set_color_format(attachment_index, mda.format);
			clear_colors[attachment_index] = {
				attClear.value.color.r,
				attClear.value.color.g,
				attClear.value.color.b,
				attClear.value.color.a
			};
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
	clear_colors[ClearAttKey::DEPTH_INDEX] = {
		depth_value,
		depth_value,
		depth_value,
		depth_value
	};

	id<MTL4RenderCommandEncoder> enc = render.encoder;

	MDResourceCache &cache = device_driver->get_resource_cache();

	[enc pushDebugGroup:@"ClearAttachments"];
	[enc setRenderPipelineState:cache.get_clear_render_pipeline_state(key, nil)];
	[enc setDepthStencilState:cache.get_depth_stencil_state(
									  key.is_depth_enabled(),
									  key.is_stencil_enabled())];
	[enc setStencilReferenceValue:stencil_value];
	[enc setCullMode:MTLCullModeNone];
	[enc setTriangleFillMode:MTLTriangleFillModeFill];
	[enc setDepthBias:0 slopeScale:0 clamp:0];
	[enc setViewport:{ 0, 0, (double)size.width, (double)size.height, 0.0, 1.0 }];
	[enc setScissorRect:{ 0, 0, (NSUInteger)size.width, (NSUInteger)size.height }];

	{
		MDRingBuffer::Allocation dst = _scratch.allocate(sizeof(clear_colors));
		memcpy(dst.ptr, clear_colors, sizeof(clear_colors));
		[_args_clear setAddress:dst.gpu_address atIndex:0];
	}

	{
		uint32_t size = vertex_count * sizeof(vertices[0]);
		MDRingBuffer::Allocation dst = _scratch.allocate(size);
		memcpy(dst.ptr, vertices, size);
		[_args_clear setAddress:dst.gpu_address atIndex:device_driver->get_metal_buffer_index_for_vertex_attribute_binding(VERT_CONTENT_BUFFER_INDEX)];
	}

	[enc setArgumentTable:_args_clear atStages:MTLRenderStageVertex | MTLRenderStageFragment];

	[enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:vertex_count];
	[enc popDebugGroup];

	render.dirty.set_flag((RenderState::DirtyFlag)(RenderState::DIRTY_PIPELINE | RenderState::DIRTY_DEPTH | RenderState::DIRTY_RASTER));
	render.mark_uniforms_dirty({ 0 }); // Mark index 0 dirty, if there is already a binding for index 0.
	render.mark_viewport_dirty();
	render.mark_scissors_dirty();
	render.mark_vertex_dirty();
	render.mark_blend_dirty();
}

void MDCommandBuffer::_render_set_dirty_state() {
	_render_bind_uniform_sets();

	id<MTLDevice> __unsafe_unretained dev = this->device_driver->get_device();

	if (render.dirty.has_flag(RenderState::DIRTY_PUSH)) {
		if (push_constant_binding != UINT32_MAX) {
			MDRingBuffer::Allocation dst = _scratch.allocate(push_constant_data_len);
			memcpy(dst.ptr, &push_constant_data, push_constant_data_len);
			[render.args setAddress:dst.gpu_address atIndex:push_constant_binding];
		}
	}

	MDSubpass const &subpass = render.get_subpass();
	if (subpass.view_count > 1) {
		MDRingBuffer::Allocation dst = _scratch.allocate(sizeof(uint32_t) * 2);
		uint32_t *view_range = (uint32_t *)dst.ptr;
		view_range[0] = 0;
		view_range[1] = subpass.view_count;

		[render.args setAddress:dst.gpu_address atIndex:VIEW_MASK_BUFFER_INDEX];
	}

	if (render.dirty.has_flag(RenderState::DIRTY_PIPELINE)) {
		[render.encoder setRenderPipelineState:render.pipeline->state];
	}

	if (render.dirty.has_flag(RenderState::DIRTY_VIEWPORT)) {
		[render.encoder setViewports:render.viewports.ptr() count:render.viewports.size()];
	}

	if (render.dirty.has_flag(RenderState::DIRTY_DEPTH)) {
		[render.encoder setDepthStencilState:render.pipeline->depth_stencil];
	}

	if (render.dirty.has_flag(RenderState::DIRTY_RASTER)) {
		render.pipeline->raster_state.apply(render.encoder);
	}

	if (render.dirty.has_flag(RenderState::DIRTY_SCISSOR) && !render.scissors.is_empty()) {
		size_t len = render.scissors.size();
		MTLScissorRect *rects = ALLOCA_ARRAY(MTLScissorRect, len);
		for (size_t i = 0; i < len; i++) {
			rects[i] = render.clip_to_render_area(render.scissors[i]);
		}
		[render.encoder setScissorRects:rects count:len];
	}

	if (render.dirty.has_flag(RenderState::DIRTY_BLEND) && render.blend_constants.has_value()) {
		[render.encoder setBlendColorRed:render.blend_constants->r green:render.blend_constants->g blue:render.blend_constants->b alpha:render.blend_constants->a];
	}

	if (render.dirty.has_flag(RenderState::DIRTY_VERTEX)) {
		uint32_t p_binding_count = render.vertex_buffers.size();
		if (p_binding_count > 0) {
			uint32_t first = device_driver->get_metal_buffer_index_for_vertex_attribute_binding(p_binding_count - 1);

			id<MTLBuffer> __unsafe_unretained *buf_ptr = render.vertex_buffers.ptr();
			NSUInteger *ofs_ptr = render.vertex_offsets.ptr();
			for (uint32_t i = first; i < first + p_binding_count; i++) {
				[render.args setAddress:(*buf_ptr).gpuAddress + *ofs_ptr atIndex:i];
				buf_ptr++;
				ofs_ptr++;
			}
		}
	}

	render.dirty.clear();

	[render.encoder setArgumentTable:render.args atStages:MTLRenderStageVertex];
	[render.encoder setArgumentTable:render.args atStages:MTLRenderStageFragment];
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
			_bind_uniforms_direct(set, shader, render.residency_set, render.args, index, dynamic_offsets);
		}
	}
}

void MDCommandBuffer::render_begin_pass(RDD::RenderPassID p_render_pass, RDD::FramebufferID p_frameBuffer, RDD::CommandBufferType p_cmd_buffer_type, const Rect2i &p_rect, VectorView<RDD::RenderPassClearValue> p_clear_values) {
	DEV_ASSERT(command_buffer != nil);
	end();

	MDRenderPass *pass = (MDRenderPass *)(p_render_pass.id);
	MDFrameBuffer *fb = (MDFrameBuffer *)(p_frameBuffer.id);

	type = MDCommandBufferStateType::Render;
	render.pass = pass;
	render.current_subpass = UINT32_MAX;
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
	DEV_ASSERT(command_buffer != nil);

	if (render.current_subpass == UINT32_MAX) {
		render.current_subpass = 0;
	} else {
		_end_render_pass();
		render.current_subpass++;
	}

	MDFrameBuffer const &fb = *render.frameBuffer;
	MDRenderPass const &pass = *render.pass;
	MDSubpass const &subpass = render.get_subpass();

	MTL4RenderPassDescriptor *desc = [MTL4RenderPassDescriptor new];

	if (subpass.view_count > 1) {
		desc.renderTargetArrayLength = subpass.view_count;
	}

	PixelFormats &pf = device_driver->get_pixel_formats();

	uint32_t attachmentCount = 0;
	for (uint32_t i = 0; i < subpass.color_references.size(); i++) {
		uint32_t idx = subpass.color_references[i].attachment;
		if (idx == RDD::AttachmentReference::UNUSED) {
			continue;
		}

		attachmentCount += 1;
		MTLRenderPassColorAttachmentDescriptor *ca = desc.colorAttachments[i];

		uint32_t resolveIdx = subpass.resolve_references.is_empty() ? RDD::AttachmentReference::UNUSED : subpass.resolve_references[i].attachment;
		bool has_resolve = resolveIdx != RDD::AttachmentReference::UNUSED;
		bool can_resolve = true;
		if (resolveIdx != RDD::AttachmentReference::UNUSED) {
			id<MTLTexture> resolve_tex = fb.get_texture(resolveIdx);
			can_resolve = flags::all(pf.getCapabilities(resolve_tex.pixelFormat), kMTLFmtCapsResolve);
			if (can_resolve) {
				ca.resolveTexture = resolve_tex;
			} else {
				CRASH_NOW_MSG("unimplemented: using a texture format that is not supported for resolve");
			}
		}

		MDAttachment const &attachment = pass.attachments[idx];

		id<MTLTexture> tex = fb.get_texture(idx);
		ERR_FAIL_NULL_MSG(tex, "Frame buffer color texture is null.");

		if ((attachment.type & MDAttachmentType::Color)) {
			if (attachment.configureDescriptor(ca, pf, subpass, tex, render.is_rendering_entire_area, has_resolve, can_resolve, false)) {
				Color clearColor = render.clear_values[idx].color;
				ca.clearColor = MTLClearColorMake(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
			}
		}
	}

	if (subpass.depth_stencil_reference.attachment != RDD::AttachmentReference::UNUSED) {
		attachmentCount += 1;
		uint32_t idx = subpass.depth_stencil_reference.attachment;
		MDAttachment const &attachment = pass.attachments[idx];
		id<MTLTexture> tex = fb.get_texture(idx);
		ERR_FAIL_NULL_MSG(tex, "Frame buffer depth / stencil texture is null.");
		if (attachment.type & MDAttachmentType::Depth) {
			MTLRenderPassDepthAttachmentDescriptor *da = desc.depthAttachment;
			if (attachment.configureDescriptor(da, pf, subpass, tex, render.is_rendering_entire_area, false, false, false)) {
				da.clearDepth = render.clear_values[idx].depth;
			}
		}

		if (attachment.type & MDAttachmentType::Stencil) {
			MTLRenderPassStencilAttachmentDescriptor *sa = desc.stencilAttachment;
			if (attachment.configureDescriptor(sa, pf, subpass, tex, render.is_rendering_entire_area, false, false, true)) {
				sa.clearStencil = render.clear_values[idx].stencil;
			}
		}
	}

	desc.renderTargetWidth = MAX((NSUInteger)MIN(render.render_area.position.x + render.render_area.size.width, fb.size.width), 1u);
	desc.renderTargetHeight = MAX((NSUInteger)MIN(render.render_area.position.y + render.render_area.size.height, fb.size.height), 1u);

	if (attachmentCount == 0) {
		// If there are no attachments, delay the creation of the encoder,
		// so we can use a matching sample count for the pipeline, by setting
		// the defaultRasterSampleCount from the pipeline's sample count.
		render.desc = desc;
	} else {
		render.encoder = [command_buffer renderCommandEncoderWithDescriptor:desc];
		_encode_barrier(render.encoder);

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

	MDSubpass const &subpass = render.get_subpass();
	if (subpass.view_count > 1) {
		p_instance_count *= subpass.view_count;
	}

	DEV_ASSERT(render.dirty == 0);

	id<MTL4RenderCommandEncoder> enc = render.encoder;

	[enc drawPrimitives:render.pipeline->raster_state.render_primitive
			  vertexStart:p_base_vertex
			  vertexCount:p_vertex_count
			instanceCount:p_instance_count
			 baseInstance:p_first_instance];
}

void MDCommandBuffer::render_bind_vertex_buffers(uint32_t p_binding_count, const RDD::BufferID *p_buffers, const uint64_t *p_offsets, uint64_t p_dynamic_offsets) {
	DEV_ASSERT(type == MDCommandBufferStateType::Render);

	render.vertex_buffers.resize(p_binding_count);
	render.vertex_offsets.resize(p_binding_count);

	// Reverse the buffers, as their bindings are assigned in descending order.
	for (uint32_t i = 0; i < p_binding_count; i += 1) {
		const RenderingDeviceDriverMetal::BufferInfo *buf_info = (const RenderingDeviceDriverMetal::BufferInfo *)p_buffers[p_binding_count - i - 1].id;

		NSUInteger dynamic_offset = 0;
		if (buf_info->is_dynamic()) {
			const MetalBufferDynamicInfo *dyn_buf = (const MetalBufferDynamicInfo *)buf_info;
			uint64_t frame_idx = p_dynamic_offsets & 0x3;
			p_dynamic_offsets >>= 2;
			dynamic_offset = frame_idx * dyn_buf->size_bytes;
		}
		render.vertex_buffers[i] = buf_info->metal_buffer;
		render.vertex_offsets[i] = dynamic_offset + p_offsets[p_binding_count - i - 1];
	}

	if (render.encoder) {
		id<MTLDevice> dev = device_driver->get_device();
		uint32_t first = device_driver->get_metal_buffer_index_for_vertex_attribute_binding(p_binding_count - 1);

		id<MTLBuffer> __unsafe_unretained *buf_ptr = render.vertex_buffers.ptr();
		NSUInteger *ofs_ptr = render.vertex_offsets.ptr();
		for (uint32_t i = first; i < first + p_binding_count; i++) {
			[render.args setAddress:(*buf_ptr).gpuAddress + *ofs_ptr atIndex:i];
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

	render.index_buffer = buffer->metal_buffer;
	render.index_type = p_format == RDD::IndexBufferFormat::INDEX_BUFFER_FORMAT_UINT16 ? MTLIndexTypeUInt16 : MTLIndexTypeUInt32;
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

	MDSubpass const &subpass = render.get_subpass();
	if (subpass.view_count > 1) {
		p_instance_count *= subpass.view_count;
	}

	id<MTL4RenderCommandEncoder> enc = render.encoder;

	uint32_t index_offset = render.index_offset;
	index_offset += p_first_index * render.index_type_size();

	[enc drawIndexedPrimitives:render.pipeline->raster_state.render_primitive
					indexCount:p_index_count
					 indexType:render.index_type
				   indexBuffer:render.index_buffer.gpuAddress + index_offset
			 // TODO(sgc): Verify length is correctly calculated
			 indexBufferLength:render.index_buffer.length - index_offset
				 instanceCount:p_instance_count
					baseVertex:p_vertex_offset
				  baseInstance:p_first_instance];
}

void MDCommandBuffer::render_draw_indexed_indirect(RDD::BufferID p_indirect_buffer, uint64_t p_offset, uint32_t p_draw_count, uint32_t p_stride) {
	DEV_ASSERT(type == MDCommandBufferStateType::Render);
	ERR_FAIL_NULL_MSG(render.pipeline, "No pipeline set for render command buffer.");

	_render_set_dirty_state();

	id<MTL4RenderCommandEncoder> enc = render.encoder;

	const RenderingDeviceDriverMetal::BufferInfo *indirect_buf = (const RenderingDeviceDriverMetal::BufferInfo *)p_indirect_buffer.id;
	id<MTLBuffer> indirect_buffer = indirect_buf->metal_buffer;
	MTLGPUAddress indirect_addr = indirect_buffer.gpuAddress + p_offset;
	MTLGPUAddress index_addr = render.index_buffer.gpuAddress;
	NSUInteger index_length = render.index_buffer.length;

	for (uint32_t i = 0; i < p_draw_count; i++) {
		[enc drawIndexedPrimitives:render.pipeline->raster_state.render_primitive
						 indexType:render.index_type
					   indexBuffer:index_addr
				 indexBufferLength:index_length
					indirectBuffer:indirect_addr];
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

	id<MTL4RenderCommandEncoder> enc = render.encoder;

	const RenderingDeviceDriverMetal::BufferInfo *indirect_buf = (const RenderingDeviceDriverMetal::BufferInfo *)p_indirect_buffer.id;
	id<MTLBuffer> indirect_buffer = indirect_buf->metal_buffer;
	MTLGPUAddress indirect_addr = indirect_buffer.gpuAddress + p_offset;

	for (uint32_t i = 0; i < p_draw_count; i++) {
		[enc drawPrimitives:render.pipeline->raster_state.render_primitive
				indirectBuffer:indirect_addr];
		indirect_addr += p_stride;
	}
}

void MDCommandBuffer::render_draw_indirect_count(RDD::BufferID p_indirect_buffer, uint64_t p_offset, RDD::BufferID p_count_buffer, uint64_t p_count_buffer_offset, uint32_t p_max_draw_count, uint32_t p_stride) {
	ERR_FAIL_MSG("not implemented");
}

void MDCommandBuffer::render_end_pass() {
	DEV_ASSERT(type == MDCommandBufferStateType::Render);

	render.end_encoding();
	render.reset();
	reset();
}

#pragma mark - RenderState

void MDCommandBuffer::RenderState::reset() {
	pass = nil;
	frameBuffer = nil;
	pipeline = nil;
	current_subpass = UINT32_MAX;
	render_area = {};
	is_rendering_entire_area = false;
	desc = nil;
	encoder = nil;
	index_buffer = nil;
	index_type = MTLIndexTypeUInt16;
	dirty = DIRTY_NONE;
	uniform_sets.clear();
	dynamic_offsets = 0;
	uniform_set_mask = 0;
	clear_values.clear();
	viewports.clear();
	scissors.clear();
	blend_constants.reset();
	bzero(vertex_buffers.ptr(), sizeof(id<MTLBuffer> __unsafe_unretained) * vertex_buffers.size());
	vertex_buffers.clear();
	bzero(vertex_offsets.ptr(), sizeof(NSUInteger) * vertex_offsets.size());
	vertex_offsets.clear();
}

void MDCommandBuffer::RenderState::end_encoding() {
	if (encoder == nil) {
		return;
	}

	[encoder endEncoding];
	encoder = nil;
}

#pragma mark - ComputeState

void MDCommandBuffer::ComputeState::end_encoding() {
	if (encoder == nil) {
		return;
	}
	[encoder endEncoding];
	encoder = nil;
}

#pragma mark - Compute

void MDCommandBuffer::_compute_set_dirty_state() {
	if (compute.dirty.has_flag(ComputeState::DIRTY_PIPELINE)) {
		DEV_ASSERT(compute.encoder == nullptr);
		compute.encoder = [command_buffer computeCommandEncoder];
		_encode_barrier(compute.encoder);
		[compute.encoder setComputePipelineState:compute.pipeline->state];
	}

	_compute_bind_uniform_sets();

	if (compute.dirty.has_flag(ComputeState::DIRTY_PUSH) && push_constant_data_len) {
		if (push_constant_binding != UINT32_MAX) {
			MDRingBuffer::Allocation dst = _scratch.allocate(push_constant_data_len);
			memcpy(dst.ptr, &push_constant_data, push_constant_data_len);
			[compute.args setAddress:dst.gpu_address atIndex:push_constant_binding];
		}
	}

	compute.dirty.clear();

	[compute.encoder setArgumentTable:compute.args];
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
			_bind_uniforms_direct(set, shader, compute.residency_set, compute.args, index, dynamic_offsets);
		}
	}
}

void MDCommandBuffer::ComputeState::reset() {
	pipeline = nil;
	encoder = nil;
	dirty = DIRTY_NONE;
	uniform_sets.clear();
	dynamic_offsets = 0;
	uniform_set_mask = 0;
}

void MDCommandBuffer::compute_bind_uniform_sets(VectorView<RDD::UniformSetID> p_uniform_sets, RDD::ShaderID p_shader, uint32_t p_first_set_index, uint32_t p_set_count, uint32_t p_dynamic_offsets) {
	DEV_ASSERT(type == MDCommandBufferStateType::Compute);

	compute.dynamic_offsets |= p_dynamic_offsets;

	if (uint32_t new_size = p_first_set_index + p_set_count; compute.uniform_sets.size() < new_size) {
		uint32_t s = compute.uniform_sets.size();
		compute.uniform_sets.resize(new_size);
		// Set intermediate values to null.
		std::fill(&compute.uniform_sets[s], compute.uniform_sets.end().operator->(), nullptr);
	}

	const MDShader *shader = (const MDShader *)p_shader.id;
	DynamicOffsetLayout layout = shader->dynamic_offset_layout;

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

	MTLRegion region = MTLRegionMake3D(0, 0, 0, p_x_groups, p_y_groups, p_z_groups);

	id<MTL4ComputeCommandEncoder> enc = compute.encoder;
	[enc dispatchThreadgroups:region.size threadsPerThreadgroup:compute.pipeline->compute_state.local];
}

void MDCommandBuffer::compute_dispatch_indirect(RDD::BufferID p_indirect_buffer, uint64_t p_offset) {
	DEV_ASSERT(type == MDCommandBufferStateType::Compute);

	_compute_set_dirty_state();

	const RenderingDeviceDriverMetal::BufferInfo *indirect_buf = (const RenderingDeviceDriverMetal::BufferInfo *)p_indirect_buffer.id;
	id<MTLBuffer> indirectBuffer = indirect_buf->metal_buffer;

	id<MTL4ComputeCommandEncoder> enc = compute.encoder;
	[enc dispatchThreadgroupsWithIndirectBuffer:indirectBuffer.gpuAddress + p_offset threadsPerThreadgroup:compute.pipeline->compute_state.local];
}

void MDCommandBuffer::reset() {
	push_constant_binding = UINT32_MAX;
	push_constant_data_len = 0;
	type = MDCommandBufferStateType::None;
}

void MDCommandBuffer::_end_compute_dispatch() {
	DEV_ASSERT(type == MDCommandBufferStateType::Compute);

	compute.end_encoding();
	compute.reset();
	reset();
}

#pragma mark - Command Pool

MDCommandBuffer *MD4CommandPool::new_command_buffer() {
	MDCommandBuffer *obj = memnew(MDCommandBuffer(_driver->get_device().newCommandAllocator, _driver));
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
	DEV_ASSERT(render.encoder != nil);

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
			RDD::BoundUniform const &uniform = p_set->uniforms[i];
			const UniformInfo &ui = shader_set.uniforms[i];
			const UniformInfo::Indexes &idx = ui.arg_buffer;

			uint32_t shift = layout.get_offset_index_shift(p_set_index, dynamic_index);
			dynamic_index++;
			uint32_t frame_idx = (p_dynamic_offsets >> shift) & 0xf;

			const MetalBufferDynamicInfo *buf_info = (const MetalBufferDynamicInfo *)uniform.ids[0].id;
			uint64_t gpu_address = buf_info->metal_buffer.gpuAddress + frame_idx * buf_info->size_bytes;
			*(MTLGPUAddress *)(ptr + idx.buffer) = gpu_address;
		}

		[render.args setAddress:alloc.gpu_address atIndex:p_set_index];
	} else {
		[render.args setAddress:p_set->arg_buffer.gpuAddress atIndex:p_set_index];
	}
}

void MDCommandBuffer::_bind_uniforms_direct(MDUniformSet *p_set, MDShader *p_shader, id<MTLResidencySet> p_rs, id<MTL4ArgumentTable> p_args, uint32_t p_set_index, uint32_t p_dynamic_offsets) {
	DEV_ASSERT(!p_shader->uses_argument_buffers);

	UniformSet const &set = p_shader->sets[p_set_index];
	DynamicOffsetLayout layout = p_shader->dynamic_offset_layout;
	uint32_t dynamic_index = 0;

	for (uint32_t i = 0; i < MIN(p_set->uniforms.size(), set.uniforms.size()); i++) {
		RDD::BoundUniform const &uniform = p_set->uniforms[i];
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
					id<MTLSamplerState> __unsafe_unretained obj = rid::get(uniform.ids[j].id);
					[p_args setSamplerState:obj.gpuResourceID atIndex:indexes.sampler + j];
				}
			} break;
			case RDD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE: {
				size_t count = uniform.ids.size() / 2;
				for (uint32_t j = 0; j < count; j += 1) {
					id<MTLSamplerState> __unsafe_unretained sampler = rid::get(uniform.ids[j * 2 + 0]);
					id<MTLTexture> __unsafe_unretained texture = rid::get(uniform.ids[j * 2 + 1]);
					[p_args setSamplerState:sampler.gpuResourceID atIndex:indexes.sampler + j];
					[p_args setTexture:texture.gpuResourceID atIndex:indexes.texture + j];
				}
			} break;
			case RDD::UNIFORM_TYPE_TEXTURE: {
				size_t count = uniform.ids.size();
				for (size_t j = 0; j < count; j += 1) {
					id<MTLTexture> obj = rid::get(uniform.ids[j]);
					[p_args setTexture:obj.gpuResourceID atIndex:indexes.texture + j];
				}
			} break;
			case RDD::UNIFORM_TYPE_IMAGE: {
				size_t count = uniform.ids.size();
				for (size_t j = 0; j < count; j += 1) {
					id<MTLTexture> obj = rid::get(uniform.ids[j]);
					[p_args setTexture:obj.gpuResourceID atIndex:indexes.texture + j];
				}

				if (indexes.buffer != UINT32_MAX) {
					// Emulated atomic image access.
					for (size_t j = 0; j < count; j += 1) {
						id<MTLTexture> obj = rid::get(uniform.ids[j]);
						id<MTLTexture> tex = obj.parentTexture ? obj.parentTexture : obj;
						id<MTLBuffer> buf = tex.buffer;
						[p_args setAddress:buf.gpuAddress + tex.bufferOffset atIndex:indexes.buffer + j];
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
				[p_args setAddress:buf_info->metal_buffer.gpuAddress atIndex:indexes.buffer];
			} break;
			case RDD::UNIFORM_TYPE_UNIFORM_BUFFER_DYNAMIC:
			case RDD::UNIFORM_TYPE_STORAGE_BUFFER_DYNAMIC: {
				const MetalBufferDynamicInfo *buf_info = (const MetalBufferDynamicInfo *)uniform.ids[0].id;
				[p_args setAddress:buf_info->metal_buffer.gpuAddress + frame_idx * buf_info->size_bytes atIndex:indexes.buffer];
			} break;
			case RDD::UNIFORM_TYPE_INPUT_ATTACHMENT: {
				size_t count = uniform.ids.size();

				for (size_t j = 0; j < count; j += 1) {
					id<MTLTexture> obj = rid::get(uniform.ids[j]);
					[p_args setTexture:obj.gpuResourceID atIndex:indexes.texture + j];
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
	DEV_ASSERT(compute.encoder != nil);

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
			RDD::BoundUniform const &uniform = p_set->uniforms[i];
			const UniformInfo &ui = shader_set.uniforms[i];
			const UniformInfo::Indexes &idx = ui.arg_buffer;

			uint32_t shift = layout.get_offset_index_shift(p_set_index, dynamic_index);
			dynamic_index++;
			uint32_t frame_idx = (p_dynamic_offsets >> shift) & 0xf;

			const MetalBufferDynamicInfo *buf_info = (const MetalBufferDynamicInfo *)uniform.ids[0].id;
			uint64_t gpu_address = buf_info->metal_buffer.gpuAddress + frame_idx * buf_info->size_bytes;
			*(MTLGPUAddress *)(ptr + idx.buffer) = gpu_address;
		}

		[compute.args setAddress:alloc.gpu_address atIndex:p_set_index];
	} else {
		[compute.args setAddress:p_set->arg_buffer.gpuAddress atIndex:p_set_index];
	}
}

static const char *SHADER_STAGE_NAMES[] = {
	[RD::SHADER_STAGE_VERTEX] = "vert",
	[RD::SHADER_STAGE_FRAGMENT] = "frag",
	[RD::SHADER_STAGE_TESSELATION_CONTROL] = "tess_ctrl",
	[RD::SHADER_STAGE_TESSELATION_EVALUATION] = "tess_eval",
	[RD::SHADER_STAGE_COMPUTE] = "comp",
};

} // namespace MTL4

GODOT_CLANG_WARNING_POP

#endif // METAL4_ENABLED
