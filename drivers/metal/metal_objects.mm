/**************************************************************************/
/*  metal_objects.mm                                                      */
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

#import "metal_objects.h"

#import "pixel_formats.h"

void MDCommandBuffer::begin() {
	DEV_ASSERT(_commandBuffer == nil);
	_commandBuffer = _queue.commandBuffer;
}

void MDCommandBuffer::end() {
	switch (type) {
		case MDCommandBufferStateType::None:
			break;
		case MDCommandBufferStateType::Render:
			render_end_pass();
			break;
		case MDCommandBufferStateType::Compute:
			endComputeDispatch();
			break;
		case MDCommandBufferStateType::Blit:
			endBlit();
			break;
	}
}

void MDCommandBuffer::commit() {
	end();
	[_commandBuffer commit];
	_commandBuffer = nil;
}

void MDCommandBuffer::bindPipeline(RDD::PipelineID pipeline) {
	MDPipeline *p = (MDPipeline *)(pipeline.id);

	// end current encoder if it is a compute encoder or blit encoder,
	// as they do not have a defined end boundary like render
	if (type == MDCommandBufferStateType::Compute) {
		endComputeDispatch();
	} else if (type == MDCommandBufferStateType::Blit) {
		endBlit();
	}

	if (p->type == MDPipelineType::Render) {
		DEV_ASSERT(type == MDCommandBufferStateType::Render);
		render.pipeline = (MDRenderPipeline *)p;
		[render.encoder setRenderPipelineState:render.pipeline->state];
		if (render.pipeline->depth_stencil != nil) {
			[render.encoder setDepthStencilState:render.pipeline->depth_stencil];
		}
		render.pipeline->raster_state.apply(render.encoder);
		render.is_dirty = false;
	} else if (p->type == MDPipelineType::Compute) {
		DEV_ASSERT(type == MDCommandBufferStateType::None);
		type = MDCommandBufferStateType::Compute;

		compute.pipeline = (MDComputePipeline *)p;
		compute.encoder = _commandBuffer.computeCommandEncoder;
		[compute.encoder setComputePipelineState:compute.pipeline->state];
	}
}

id<MTLBlitCommandEncoder> MDCommandBuffer::blitCommandEncoder() {
	switch (type) {
		case MDCommandBufferStateType::None:
			break;
		case MDCommandBufferStateType::Render:
			render_end_pass();
			break;
		case MDCommandBufferStateType::Compute:
			endComputeDispatch();
			break;
		case MDCommandBufferStateType::Blit:
			return blit.encoder;
	}

	type = MDCommandBufferStateType::Blit;
	blit.encoder = _commandBuffer.blitCommandEncoder;
	return blit.encoder;
}

void MDCommandBuffer::encodeRenderCommandEncoderWithDescriptor(MTLRenderPassDescriptor *desc, NSString *label) {
	switch (type) {
		case MDCommandBufferStateType::None:
			break;
		case MDCommandBufferStateType::Render:
			render_end_pass();
			break;
		case MDCommandBufferStateType::Compute:
			endComputeDispatch();
			break;
		case MDCommandBufferStateType::Blit:
			endBlit();
			break;
	}

	id<MTLRenderCommandEncoder> enc = [_commandBuffer renderCommandEncoderWithDescriptor:desc];
	if (label != nil) {
		[enc pushDebugGroup:label];
		[enc popDebugGroup];
	}
	[enc endEncoding];
}

#pragma mark - Render Commands

void MDCommandBuffer::render_begin_pass(RDD::RenderPassID p_render_pass, RDD::FramebufferID p_frameBuffer, RDD::CommandBufferType p_cmd_buffer_type, const Rect2i &p_rect, VectorView<RDD::RenderPassClearValue> p_clear_values) {
	DEV_ASSERT(_commandBuffer != nil);
	end();

	MDRenderPass *pass = (MDRenderPass *)(p_render_pass.id);
	MDFrameBuffer *fb = (MDFrameBuffer *)(p_frameBuffer.id);

	MTLRenderPassDescriptor *desc = fb->newRenderPassDescriptorWithRenderPass(pass, p_clear_values);
	desc.renderTargetWidth = p_rect.size.width;
	desc.renderTargetHeight = p_rect.size.height;

	type = MDCommandBufferStateType::Render;
	render.pass = pass;
	render.frameBuffer = fb;
	render.encoder = [_commandBuffer renderCommandEncoderWithDescriptor:desc];
}

void MDCommandBuffer::render_draw(uint32_t p_vertex_count,
		uint32_t p_instance_count,
		uint32_t p_base_vertex,
		uint32_t p_first_instance) const {
	DEV_ASSERT(type == MDCommandBufferStateType::Render);

	id<MTLRenderCommandEncoder> enc = render.encoder;

	[enc drawPrimitives:render.pipeline->raster_state.render_primitive
			  vertexStart:p_base_vertex
			  vertexCount:p_vertex_count
			instanceCount:p_instance_count
			 baseInstance:p_first_instance];
}

void MDCommandBuffer::render_bind_index_buffer(RDD::BufferID p_buffer, RDD::IndexBufferFormat p_format, uint64_t p_offset) {
	DEV_ASSERT(type == MDCommandBufferStateType::Render);

	render.index_buffer = rid::get(p_buffer);
	render.index_type = p_format == RDD::IndexBufferFormat::INDEX_BUFFER_FORMAT_UINT16 ? MTLIndexTypeUInt16 : MTLIndexTypeUInt32;
}

void MDCommandBuffer::render_draw_indexed(uint32_t p_index_count,
		uint32_t p_instance_count,
		uint32_t p_first_index,
		int32_t p_vertex_offset,
		uint32_t p_first_instance) const {
	DEV_ASSERT(type == MDCommandBufferStateType::Render);

	id<MTLRenderCommandEncoder> enc = render.encoder;

	[enc drawIndexedPrimitives:render.pipeline->raster_state.render_primitive
					indexCount:p_index_count
					 indexType:render.index_type
				   indexBuffer:render.index_buffer
			 indexBufferOffset:p_vertex_offset
				 instanceCount:p_instance_count
					baseVertex:p_first_index
				  baseInstance:p_first_instance];
}

void MDCommandBuffer::render_end_pass() {
	DEV_ASSERT(type == MDCommandBufferStateType::Render);

	[render.encoder endEncoding];
	render = {};
	type = MDCommandBufferStateType::None;
}

void MDCommandBuffer::endComputeDispatch() {
	DEV_ASSERT(type == MDCommandBufferStateType::Compute);

	[compute.encoder endEncoding];
	compute = {};
	type = MDCommandBufferStateType::None;
}

void MDCommandBuffer::endBlit() {
	DEV_ASSERT(type == MDCommandBufferStateType::Blit);

	[blit.encoder endEncoding];
	blit = {};
	type = MDCommandBufferStateType::None;
}

MDComputeShader::MDComputeShader(String p_name, Vector<UniformSet> p_sets, id<MTLLibrary> p_kernel) :
		MDShader(p_name, p_sets), kernel(p_kernel) {
}

void MDComputeShader::encodePushConstantData(VectorView<uint32_t> data, MDCommandBuffer *cb) {
	DEV_ASSERT(cb->type == MDCommandBufferStateType::Compute);
	id<MTLComputeCommandEncoder> enc = cb->compute.encoder;

	void const *ptr = data.ptr();
	size_t length = data.size() * sizeof(uint32_t);

	[enc setBytes:ptr length:length atIndex:push_constants.binding];
}

MDRenderShader::MDRenderShader(String p_name, Vector<UniformSet> p_sets, id<MTLLibrary> _Nonnull p_vert, id<MTLLibrary> _Nonnull p_frag) :
		MDShader(p_name, p_sets), vert(p_vert), frag(p_frag) {
}

void MDRenderShader::encodePushConstantData(VectorView<uint32_t> data, MDCommandBuffer *cb) {
	DEV_ASSERT(cb->type == MDCommandBufferStateType::Render);
	id<MTLRenderCommandEncoder> enc = cb->render.encoder;

	void const *ptr = data.ptr();
	size_t length = data.size() * sizeof(uint32_t);

	if (push_constants.vert.binding > -1) {
		[enc setVertexBytes:ptr length:length atIndex:push_constants.vert.binding];
	}

	if (push_constants.frag.binding > -1) {
		[enc setFragmentBytes:ptr length:length atIndex:push_constants.frag.binding];
	}
}

BoundUniformSet &MDUniformSet::boundUniformSetForShader(RDD::ShaderID p_shader, id<MTLDevice> device) {
	BoundUniformSet *sus = bound_uniforms.getptr(p_shader);
	if (sus != nullptr) {
		return *sus;
	}

	MDShader *shader = (MDShader *)(p_shader.id);
	UniformSet const &set = shader->sets[index];

	HashMap<id<MTLResource>, StageResourceUsage> bound_resources;
	auto add_usage = [&bound_resources](id<MTLResource> __unsafe_unretained res, RDD::ShaderStage stage, MTLResourceUsage usage) {
		StageResourceUsage *sru = bound_resources.getptr(res);
		if (sru == nullptr) {
			bound_resources.insert(res, stage_resource_usage(stage, usage));
		} else {
			*sru |= stage_resource_usage(stage, usage);
		}
	};
	id<MTLBuffer> enc_buffer = nil;
	if (set.buffer_size > 0) {
		MTLResourceOptions options = MTLResourceStorageModeShared | MTLResourceHazardTrackingModeTracked;
		enc_buffer = [device newBufferWithLength:set.buffer_size options:options];
		for (auto &kv : set.encoders) {
			RDD::ShaderStage const stage = kv.key;
			ShaderStageUsage const stage_usage = ShaderStageUsage(1 << stage);
			id<MTLArgumentEncoder> const enc = kv.value;

			[enc setArgumentBuffer:enc_buffer offset:set.offsets[stage]];

			for (uint32_t i = 0; i < p_uniforms.size(); i++) {
				RDD::BoundUniform const &uniform = p_uniforms[i];
				UniformInfo ui = set.uniforms[i];

				// no binding for this stage
				BindingInfo *bi = ui.bindings.getptr(stage);
				if (bi == nullptr)
					continue;

				if ((ui.active_stages & stage_usage) == 0) {
					// not active for this state, so don't bind anything
					continue;
				}

				switch (uniform.type) {
					case RDD::UNIFORM_TYPE_SAMPLER: {
						size_t count = uniform.ids.size();
						id<MTLSamplerState> __unsafe_unretained *objects = ALLOCA_ARRAY(id<MTLSamplerState> __unsafe_unretained, count);
						for (size_t j = 0; j < count; j += 1) {
							objects[j] = rid::get(uniform.ids[j].id);
						}
						[enc setSamplerStates:objects withRange:NSMakeRange(bi->index, count)];
					} break;
					case RDD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE: {
						size_t count = uniform.ids.size() / 2;
						id<MTLTexture> __unsafe_unretained *textures = ALLOCA_ARRAY(id<MTLTexture> __unsafe_unretained, count);
						id<MTLSamplerState> __unsafe_unretained *samplers = ALLOCA_ARRAY(id<MTLSamplerState> __unsafe_unretained, count);
						for (int j = 0; j < count; j += 1) {
							id<MTLSamplerState> sampler = rid::get(uniform.ids[j * 2 + 0]);
							id<MTLTexture> texture = rid::get(uniform.ids[j * 2 + 1]);
							samplers[j] = sampler;
							textures[j] = texture;
							add_usage(texture, stage, bi->usage);
						}
						BindingInfo *sbi = ui.bindings_secondary.getptr(stage);
						if (sbi) {
							[enc setSamplerStates:samplers withRange:NSMakeRange(sbi->index, count)];
						}
						[enc setTextures:textures
								withRange:NSMakeRange(bi->index, count)];
					} break;
					case RDD::UNIFORM_TYPE_TEXTURE: {
						size_t count = uniform.ids.size();
						if (count == 1) {
							id<MTLTexture> obj = rid::get(uniform.ids[0]);
							[enc setTexture:obj atIndex:bi->index];
							add_usage(obj, stage, bi->usage);
						} else {
							id<MTLTexture> __unsafe_unretained *objects = ALLOCA_ARRAY(id<MTLTexture> __unsafe_unretained, count);
							for (size_t j = 0; j < count; j += 1) {
								id<MTLTexture> obj = rid::get(uniform.ids[j]);
								objects[j] = obj;
								add_usage(obj, stage, bi->usage);
							}
							[enc setTextures:objects withRange:NSMakeRange(bi->index, count)];
						}
					} break;
					case RDD::UNIFORM_TYPE_IMAGE: {
						size_t count = uniform.ids.size();
						if (count == 1) {
							id<MTLTexture> obj = rid::get(uniform.ids[0]);
							[enc setTexture:obj atIndex:bi->index];
							add_usage(obj, stage, bi->usage);
						} else {
							id<MTLTexture> __unsafe_unretained *objects = ALLOCA_ARRAY(id<MTLTexture> __unsafe_unretained, count);
							for (size_t j = 0; j < count; j += 1) {
								id<MTLTexture> obj = rid::get(uniform.ids[j]);
								objects[j] = obj;
								add_usage(obj, stage, bi->usage);
							}
							[enc setTextures:objects withRange:NSMakeRange(bi->index, count)];
						}
					} break;
					case RDD::UNIFORM_TYPE_TEXTURE_BUFFER: {
						ERR_PRINT("not implemented: UNIFORM_TYPE_TEXTURE_BUFFER");
					} break;
					case RDD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE_BUFFER: {
						ERR_PRINT("not implemented: UNIFORM_TYPE_SAMPLER_WITH_TEXTURE_BUFFER");
					} break;
					case RDD::UNIFORM_TYPE_IMAGE_BUFFER: {
						CRASH_NOW_MSG("Unimplemented!"); // TODO.
					} break;
					case RDD::UNIFORM_TYPE_UNIFORM_BUFFER: {
						id<MTLBuffer> buffer = rid::get(uniform.ids[0]);
						[enc setBuffer:buffer offset:0 atIndex:bi->index];
						add_usage(buffer, stage, bi->usage);
					} break;
					case RDD::UNIFORM_TYPE_STORAGE_BUFFER: {
						id<MTLBuffer> buffer = rid::get(uniform.ids[0]);
						[enc setBuffer:buffer offset:0 atIndex:bi->index];
						add_usage(buffer, stage, bi->usage);
					} break;
					case RDD::UNIFORM_TYPE_INPUT_ATTACHMENT: {
						ERR_PRINT("not implemented: UNIFORM_TYPE_INPUT_ATTACHMENT");
					} break;
					default: {
						DEV_ASSERT(false);
					}
				}
			}
		}
	}

	BoundUniformSet bs = { .buffer = enc_buffer, .bound_resources = bound_resources };
	bound_uniforms.insert(p_shader, bs);
	return bound_uniforms.get(p_shader);
}

MTLRenderPassDescriptor *MDFrameBuffer::newRenderPassDescriptorWithRenderPass(MDRenderPass *pass, VectorView<RDD::RenderPassClearValue> colors) const {
	MTLRenderPassDescriptor *desc = MTLRenderPassDescriptor.renderPassDescriptor;
	if (pass->_attachments.is_empty()) {
		desc.defaultRasterSampleCount = 4;
		return desc;
	}
	for (int i = 0; i < pass->_attachments.size(); i++) {
		MDAttachment const &attachment = pass->_attachments[i];
		id<MTLTexture> tex = textures[i];
		if ((attachment.type & MDAttachmentType::Color)) {
			MTLRenderPassColorAttachmentDescriptor *ca = desc.colorAttachments[i];
			ca.texture = tex;
			ca.loadAction = attachment.loadAction;
			ca.storeAction = attachment.storeAction;
			Color clearColor = colors[i].color;
			ca.clearColor = MTLClearColorMake(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		}

		if (attachment.type & MDAttachmentType::Depth) {
			desc.depthAttachment.texture = tex;
			desc.depthAttachment.loadAction = attachment.loadAction;
			desc.depthAttachment.storeAction = attachment.storeAction;
			desc.depthAttachment.clearDepth = colors[i].depth;
		}

		if (attachment.type & MDAttachmentType::Stencil) {
			desc.stencilAttachment.texture = tex;
			desc.stencilAttachment.loadAction = attachment.loadAction;
			desc.stencilAttachment.storeAction = attachment.storeAction;
			desc.stencilAttachment.clearStencil = colors[i].stencil;
		}
	}
	return desc;
}

std::shared_ptr<MDQueryPool> MDQueryPool::newQueryPool(id<MTLDevice> device, NSError **error) {
	std::shared_ptr<MDQueryPool> pool(new MDQueryPool());

	// Set the sample count to 4, to make room for the:
	// – Vertex stage's start time
	// – Vertex stage's completion time
	// – Fragment stage's start time
	// – Fragment stage's completion time
	pool->sampleCount = 4;

	for (id<MTLCounterSet> cs in device.counterSets) {
		NSString *csName = cs.name;
		if ([cs.name isEqualToString:MTLCommonCounterSetTimestamp]) {
			for (id<MTLCounter> ctr in cs.counters) {
				if ([ctr.name isEqualToString:MTLCommonCounterTimestamp]) {
					pool->counterSet = cs;
					break;
				}
			}
			break;
		}
	}

	if (pool->counterSet == nil)
		return nil;

	@autoreleasepool {
		MTLCounterSampleBufferDescriptor *desc = [MTLCounterSampleBufferDescriptor new];
		desc.counterSet = pool->counterSet;
		desc.storageMode = MTLStorageModeShared;
		desc.sampleCount = pool->sampleCount;

		pool->_counterSampleBuffer = [device newCounterSampleBufferWithDescriptor:desc error:error];
		if (*error) {
			return nil;
		}
	}

	pool->results.resize(2);

	return pool;
}

void MDQueryPool::resetWithCommandBuffer(RDD::CommandBufferID p_cmd_buffer) {
	MDCommandBuffer *cb = (MDCommandBuffer *)(p_cmd_buffer.id);

	MTLTimestamp cpuStartTimestamp, gpuStartTimestamp;
	[cb->get_command_buffer().device sampleTimestamps:&cpuStartTimestamp gpuTimestamp:&gpuStartTimestamp];
	cpuStart = (double)cpuStartTimestamp;
	gpuStart = (double)gpuStartTimestamp;

	[cb->get_command_buffer() addCompletedHandler:^(id<MTLCommandBuffer> _Nonnull commandBuffer) {
	  CFTimeInterval base = commandBuffer.GPUStartTime;
	  results[0] = 0.0f;
	  results[1] = (commandBuffer.GPUEndTime - base) * 1'000'000.0; // to µs

	  MTLTimestamp cpuEndTimestamp, gpuEndTimestamp;
	  [commandBuffer.device sampleTimestamps:&cpuEndTimestamp gpuTimestamp:&gpuEndTimestamp];
	  cpuTimeSpan = (double)cpuEndTimestamp - cpuStart;
	  gpuTimeSpan = (double)gpuEndTimestamp - gpuStart;
	  resolveSampleBuffer();
	}];
}

void MDQueryPool::getResults(uint64_t *_Nonnull p_results, NSUInteger count) {
	DEV_ASSERT(count <= results.size());
	for (int i = 0; i < count; i++) {
		p_results[i] = (uint64_t)results[i];
	}
}

void MDQueryPool::writeCommandBuffer(RDD::CommandBufferID p_cmd_buffer, NSUInteger index) {
	MDCommandBuffer *cb = (MDCommandBuffer *)(p_cmd_buffer.id);

	switch (cb->type) {
		case MDCommandBufferStateType::None:
			return;
		case MDCommandBufferStateType::Render:
			[cb->render.encoder sampleCountersInBuffer:_counterSampleBuffer atSampleIndex:0 withBarrier:NO];
			break;
		case MDCommandBufferStateType::Compute:
			[cb->compute.encoder sampleCountersInBuffer:_counterSampleBuffer atSampleIndex:0 withBarrier:NO];
			break;
		case MDCommandBufferStateType::Blit:
			// TODO(sgc): Must check MTLCounterSamplingPointAtBlitBoundary
			// [cb->blit.encoder sampleCountersInBuffer:_counterSampleBuffer atSampleIndex:0 withBarrier:NO];
			break;
	}
}

void MDQueryPool::resolveSampleBuffer() {
	/// Represents the size of the counter sample buffer.
	NSRange range = NSMakeRange(0, sampleCount);

	// Convert the contents of the counter sample buffer into the standard data format.
	NSData *data = [_counterSampleBuffer resolveCounterRange:range];
	if (nil == data) {
		return;
	}

	NSUInteger resolvedSampleCount = data.length / sizeof(MTLCounterResultTimestamp);
	if (resolvedSampleCount < sampleCount) {
		return;
	}

	// Cast the data's bytes property to the counter's result type.
	auto timestamps = (MTLCounterResultTimestamp *)(data.bytes);

	// the first two entries are always GPUStartTime and GPUEndTime
	results.resize(resolvedSampleCount + 2);
	double *results_ptr = results.ptr() + 2;

	// Check for invalid values within the (resolved) data from the counter sample buffer.
	for (int index = 0; index < resolvedSampleCount; index++) {
		MTLTimestamp timestamp = timestamps[index].timestamp;

		if (timestamp == MTLCounterErrorValue) {
			return;
		}

		if (timestamp == 0) {
			return;
		}

		// Convert the GPU time to a value within the range [0.0, 1.0].
		double normalizedGpuTime = ((double)timestamp - gpuStart);
		normalizedGpuTime /= gpuTimeSpan;

		// Convert GPU time to CPU time.
		double nanoseconds = (normalizedGpuTime * cpuTimeSpan);
		nanoseconds += cpuStart;

		double microseconds = nanoseconds / 1000.0;
		results_ptr[index] = microseconds;
	}
}

#pragma mark - Resource Factory

id<MTLFunction> MDResourceFactory::new_func(NSString *p_source, NSString *p_name, NSError **p_error) {
	@autoreleasepool {
		id<MTLFunction> mtlFunc = nil;
		NSError *err = nil;
		MTLCompileOptions *options = [MTLCompileOptions new];
		id<MTLLibrary> mtlLib = [context->get_device() newLibraryWithSource:p_source
																	options:options
																	  error:&err]; // temp retain
		if (err) {
			if (p_error != nil) {
				*p_error = err;
			}
		}
		return [mtlLib newFunctionWithName:p_name];
	}
}

id<MTLFunction> MDResourceFactory::new_clear_vert_func(ClearAttKey &p_key) {
	@autoreleasepool {
		NSString *msl = [NSString stringWithFormat:@R"(
#include <metal_stdlib>
using namespace metal;

typedef struct {
    float4 a_position [[attribute(0)]];
} AttributesPos;

typedef struct {
    float4 colors[9];
} ClearColorsIn;

typedef struct {
    float4 v_position [[position]];
    uint layer;
} VaryingsPos;

vertex VaryingsPos vertClear(AttributesPos attributes [[stage_in]], constant ClearColorsIn& ccIn [[buffer(0)]]) {
    VaryingsPos varyings;
    varyings.v_position = float4(attributes.a_position.x, -attributes.a_position.y, ccIn.colors[%d].r, 1.0);
    varyings.layer = uint(attributes.a_position.w);
    return varyings;
}
)",
								  ClearAttKey::DEPTH_INDEX];

		return new_func(msl, @"vertClear", nil);
	}
}

id<MTLFunction> MDResourceFactory::new_clear_frag_func(ClearAttKey &p_key) {
	@autoreleasepool {
		NSMutableString *msl = [NSMutableString stringWithCapacity:(2 * KIBI)];

		[msl appendFormat:@R"(
#include <metal_stdlib>
using namespace metal;

typedef struct {
    float4 v_position [[position]];
} VaryingsPos;

typedef struct {
    float4 colors[9];
} ClearColorsIn;

typedef struct {
)"];

		for (uint32_t caIdx = 0; caIdx < ClearAttKey::COLOR_COUNT; caIdx++) {
			if (p_key.is_enabled(caIdx)) {
				NSString *typeStr = get_format_type_string((MTLPixelFormat)p_key.pixel_formats[caIdx]);
				[msl appendFormat:@"    %@4 color%u [[color(%u)]];\n", typeStr, caIdx, caIdx];
			}
		}
		[msl appendFormat:@R"(} ClearColorsOut;

fragment ClearColorsOut fragClear(VaryingsPos varyings [[stage_in]], constant ClearColorsIn& ccIn [[buffer(0)]]) {

    ClearColorsOut ccOut;
)"];
		for (uint32_t caIdx = 0; caIdx < ClearAttKey::COLOR_COUNT; caIdx++) {
			if (p_key.is_enabled(caIdx)) {
				NSString *typeStr = get_format_type_string((MTLPixelFormat)p_key.pixel_formats[caIdx]);
				[msl appendFormat:@"    ccOut.color%u = %@4(ccIn.colors[%u]);\n", caIdx, typeStr, caIdx];
			}
		}
		[msl appendString:@R"(    return ccOut;
})"];

		return new_func(msl, @"fragClear", nil);
	}
}

NSString *MDResourceFactory::get_format_type_string(MTLPixelFormat p_fmt) {
	switch (context->get_pixel_formats().getFormatType(p_fmt)) {
		case kMVKFormatColorInt8:
		case kMVKFormatColorInt16:
			return @"short";
		case kMVKFormatColorUInt8:
		case kMVKFormatColorUInt16:
			return @"ushort";
		case kMVKFormatColorInt32:
			return @"int";
		case kMVKFormatColorUInt32:
			return @"uint";
		case kMVKFormatColorHalf:
			return @"half";
		case kMVKFormatColorFloat:
		case kMVKFormatDepthStencil:
		case kMVKFormatCompressed:
			return @"float";
		case kMVKFormatNone:
			return @"unexpected_MTLPixelFormatInvalid";
	}
}

id<MTLDepthStencilState> MDResourceFactory::new_depth_stencil_state(bool p_use_depth, bool p_use_stencil) {
	MTLDepthStencilDescriptor *dsDesc = [MTLDepthStencilDescriptor new]; // temp retain
	dsDesc.depthCompareFunction = MTLCompareFunctionAlways;
	dsDesc.depthWriteEnabled = p_use_depth;

	if (p_use_stencil) {
		MTLStencilDescriptor *sDesc = [MTLStencilDescriptor new]; // temp retain
		sDesc.stencilCompareFunction = MTLCompareFunctionAlways;
		sDesc.stencilFailureOperation = MTLStencilOperationReplace;
		sDesc.depthFailureOperation = MTLStencilOperationReplace;
		sDesc.depthStencilPassOperation = MTLStencilOperationReplace;

		dsDesc.frontFaceStencil = sDesc;
		dsDesc.backFaceStencil = sDesc;
	} else {
		dsDesc.frontFaceStencil = nil;
		dsDesc.backFaceStencil = nil;
	}

	id<MTLDepthStencilState> dss = [context->get_device() newDepthStencilStateWithDescriptor:dsDesc];

	return dss;
}

id<MTLRenderPipelineState> MDResourceFactory::new_clear_pipeline_state(ClearAttKey &p_key, NSError **p_error) {
	PixelFormats &pixFmts = context->get_pixel_formats();

	id<MTLFunction> vtxFunc = new_clear_vert_func(p_key); // temp retain
	id<MTLFunction> fragFunc = new_clear_frag_func(p_key); // temp retain
	MTLRenderPipelineDescriptor *plDesc = [MTLRenderPipelineDescriptor new]; // temp retain
	plDesc.label = @"ClearRenderAttachments";
	plDesc.vertexFunction = vtxFunc;
	plDesc.fragmentFunction = fragFunc;
	plDesc.rasterSampleCount = p_key.sample_count;
	plDesc.inputPrimitiveTopology = MTLPrimitiveTopologyClassTriangle;

	for (uint32_t caIdx = 0; caIdx < ClearAttKey::COLOR_COUNT; caIdx++) {
		MTLRenderPipelineColorAttachmentDescriptor *colorDesc = plDesc.colorAttachments[caIdx];
		colorDesc.pixelFormat = (MTLPixelFormat)p_key.pixel_formats[caIdx];
		colorDesc.writeMask = p_key.is_enabled(caIdx) ? MTLColorWriteMaskAll : MTLColorWriteMaskNone;
	}

	MTLPixelFormat mtlDepthFormat = p_key.depth_format();
	if (pixFmts.isDepthFormat(mtlDepthFormat)) {
		plDesc.depthAttachmentPixelFormat = mtlDepthFormat;
	}

	MTLPixelFormat mtlStencilFormat = p_key.stencil_format();
	if (pixFmts.isStencilFormat(mtlStencilFormat)) {
		plDesc.stencilAttachmentPixelFormat = mtlStencilFormat;
	}

	MTLVertexDescriptor *vtxDesc = plDesc.vertexDescriptor;

	// Vertex attribute descriptors
	MTLVertexAttributeDescriptorArray *vaDescArray = vtxDesc.attributes;
	MTLVertexAttributeDescriptor *vaDesc;
	NSUInteger vtxBuffIdx = context->get_metal_buffer_index_for_vertex_attribute_binding(VERT_CONTENT_BUFFER_INDEX);
	NSUInteger vtxStride = 0;

	// Vertex location
	vaDesc = vaDescArray[0];
	vaDesc.format = MTLVertexFormatFloat4;
	vaDesc.bufferIndex = vtxBuffIdx;
	vaDesc.offset = vtxStride;
	vtxStride += sizeof(simd::float4);

	// Vertex attribute buffer.
	MTLVertexBufferLayoutDescriptorArray *vbDescArray = vtxDesc.layouts;
	MTLVertexBufferLayoutDescriptor *vbDesc = vbDescArray[vtxBuffIdx];
	vbDesc.stepFunction = MTLVertexStepFunctionPerVertex;
	vbDesc.stepRate = 1;
	vbDesc.stride = vtxStride;

	id<MTLRenderPipelineState> rps = [device newRenderPipelineStateWithDescriptor:plDesc error:p_error];

	return rps;
}
