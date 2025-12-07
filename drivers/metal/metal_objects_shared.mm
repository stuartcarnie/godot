/**************************************************************************/
/*  metal_objects_shared.mm                                               */
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

#import "metal_objects_shared.h"

#import <simd/simd.h>

#pragma mark - Resource Factory

id<MTLFunction> MDResourceFactory::new_func(NSString *p_source, NSString *p_name, NSError **p_error) {
	@autoreleasepool {
		NSError *err = nil;
		MTLCompileOptions *options = [MTLCompileOptions new];
		id<MTLLibrary> mtlLib = [device newLibraryWithSource:p_source
													 options:options
													   error:&err];
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
    uint layer%s;
} VaryingsPos;

vertex VaryingsPos vertClear(AttributesPos attributes [[stage_in]], constant ClearColorsIn& ccIn [[buffer(0)]]) {
    VaryingsPos varyings;
    varyings.v_position = float4(attributes.a_position.x, -attributes.a_position.y, ccIn.colors[%d].r, 1.0);
    varyings.layer = uint(attributes.a_position.w);
    return varyings;
}
)", p_key.is_layered_rendering_enabled() ? " [[render_target_array_index]]" : "", ClearAttKey::DEPTH_INDEX];

		return new_func(msl, @"vertClear", nil);
	}
}

id<MTLFunction> MDResourceFactory::new_clear_frag_func(ClearAttKey &p_key) {
	@autoreleasepool {
		NSMutableString *msl = [NSMutableString stringWithCapacity:2048];

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
	switch (pixel_formats.getFormatType(p_fmt)) {
		case MTLFormatType::ColorInt8:
		case MTLFormatType::ColorInt16:
			return @"short";
		case MTLFormatType::ColorUInt8:
		case MTLFormatType::ColorUInt16:
			return @"ushort";
		case MTLFormatType::ColorInt32:
			return @"int";
		case MTLFormatType::ColorUInt32:
			return @"uint";
		case MTLFormatType::ColorHalf:
			return @"half";
		case MTLFormatType::ColorFloat:
		case MTLFormatType::DepthStencil:
		case MTLFormatType::Compressed:
			return @"float";
		case MTLFormatType::None:
			return @"unexpected_MTLPixelFormatInvalid";
	}
}

id<MTLDepthStencilState> MDResourceFactory::new_depth_stencil_state(bool p_use_depth, bool p_use_stencil) {
	MTLDepthStencilDescriptor *dsDesc = [MTLDepthStencilDescriptor new];
	dsDesc.depthCompareFunction = MTLCompareFunctionAlways;
	dsDesc.depthWriteEnabled = p_use_depth;

	if (p_use_stencil) {
		MTLStencilDescriptor *sDesc = [MTLStencilDescriptor new];
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

	return [device newDepthStencilStateWithDescriptor:dsDesc];
}

id<MTLRenderPipelineState> MDResourceFactory::new_clear_pipeline_state(ClearAttKey &p_key, NSError **p_error) {
	id<MTLFunction> vtxFunc = new_clear_vert_func(p_key);
	id<MTLFunction> fragFunc = new_clear_frag_func(p_key);
	MTLRenderPipelineDescriptor *plDesc = [MTLRenderPipelineDescriptor new];
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
	if (pixel_formats.isDepthFormat(mtlDepthFormat)) {
		plDesc.depthAttachmentPixelFormat = mtlDepthFormat;
	}

	MTLPixelFormat mtlStencilFormat = p_key.stencil_format();
	if (pixel_formats.isStencilFormat(mtlStencilFormat)) {
		plDesc.stencilAttachmentPixelFormat = mtlStencilFormat;
	}

	MTLVertexDescriptor *vtxDesc = plDesc.vertexDescriptor;

	// Vertex attribute descriptors.
	MTLVertexAttributeDescriptorArray *vaDescArray = vtxDesc.attributes;
	MTLVertexAttributeDescriptor *vaDesc;
	NSUInteger vtxBuffIdx = get_vertex_buffer_index(VERT_CONTENT_BUFFER_INDEX);
	NSUInteger vtxStride = 0;

	// Vertex location.
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

	return [device newRenderPipelineStateWithDescriptor:plDesc error:p_error];
}

id<MTLRenderPipelineState> MDResourceFactory::new_empty_draw_pipeline_state(ClearAttKey &p_key, NSError **p_error) {
	DEV_ASSERT(!p_key.is_layered_rendering_enabled());
	DEV_ASSERT(p_key.is_enabled(0));
	DEV_ASSERT(!p_key.is_depth_enabled());
	DEV_ASSERT(!p_key.is_stencil_enabled());

	@autoreleasepool {
		NSMutableString *msl = [NSMutableString stringWithCapacity:512];
		[msl appendString:@"#include <metal_stdlib>\nusing namespace metal;\n\n"];
		[msl appendString:@"struct FullscreenNoopOut {\n    float4 position [[position]];\n};\n\n"];
		[msl appendString:@"vertex FullscreenNoopOut fullscreenNoopVert(uint vid [[vertex_id]]) {\n"];
		[msl appendString:@"    float2 positions[3] = { float2(-1.0, -1.0), float2(3.0, -1.0), float2(-1.0, 3.0) };\n"];
		[msl appendString:@"    float2 pos = positions[vid];\n\n"];
		[msl appendString:@"    FullscreenNoopOut out;\n"];
		[msl appendString:@"    out.position = float4(pos, 0.0, 1.0);\n"];
		[msl appendString:@"    return out;\n"];
		[msl appendString:@"}\n\nfragment void fullscreenNoopFrag(float4 gl_FragCoord [[position]]) {\n}\n"];

		NSError *err = nil;
		MTLCompileOptions *options = [MTLCompileOptions new];
		id<MTLLibrary> mtlLib = [device newLibraryWithSource:msl options:options error:&err];
		if (err && p_error != nil) {
			*p_error = err;
		}

		if (mtlLib == nil) {
			return nil;
		}

		id<MTLFunction> vtxFunc = [mtlLib newFunctionWithName:@"fullscreenNoopVert"];
		id<MTLFunction> fragFunc = [mtlLib newFunctionWithName:@"fullscreenNoopFrag"];

		MTLRenderPipelineDescriptor *plDesc = [MTLRenderPipelineDescriptor new];
		plDesc.label = @"EmptyDrawFullscreenTriangle";
		plDesc.vertexFunction = vtxFunc;
		plDesc.fragmentFunction = fragFunc;
		plDesc.rasterSampleCount = p_key.sample_count ? p_key.sample_count : 1;
		plDesc.inputPrimitiveTopology = MTLPrimitiveTopologyClassTriangle;

		MTLRenderPipelineColorAttachmentDescriptor *colorDesc = plDesc.colorAttachments[0];
		colorDesc.pixelFormat = (MTLPixelFormat)p_key.pixel_formats[0];
		colorDesc.writeMask = MTLColorWriteMaskNone;

		return [device newRenderPipelineStateWithDescriptor:plDesc error:p_error];
	}
}

#pragma mark - Resource Cache

id<MTLRenderPipelineState> MDResourceCache::get_clear_render_pipeline_state(ClearAttKey &p_key, NSError **p_error) {
	HashMap::ConstIterator it = clear_states.find(p_key);
	if (it != clear_states.end()) {
		return it->value;
	}

	id<MTLRenderPipelineState> state = resource_factory->new_clear_pipeline_state(p_key, p_error);
	clear_states[p_key] = state;
	return state;
}

id<MTLRenderPipelineState> MDResourceCache::get_empty_draw_pipeline_state(ClearAttKey &p_key, NSError **p_error) {
	HashMap::ConstIterator it = empty_draw_states.find(p_key);
	if (it != empty_draw_states.end()) {
		return it->value;
	}

	id<MTLRenderPipelineState> state = resource_factory->new_empty_draw_pipeline_state(p_key, p_error);
	empty_draw_states[p_key] = state;
	return state;
}

id<MTLDepthStencilState> MDResourceCache::get_depth_stencil_state(bool p_use_depth, bool p_use_stencil) {
	if (p_use_depth && p_use_stencil) {
		if (clear_depth_stencil_state.all == nil) {
			clear_depth_stencil_state.all = resource_factory->new_depth_stencil_state(true, true);
		}
		return clear_depth_stencil_state.all;
	} else if (p_use_depth) {
		if (clear_depth_stencil_state.depth_only == nil) {
			clear_depth_stencil_state.depth_only = resource_factory->new_depth_stencil_state(true, false);
		}
		return clear_depth_stencil_state.depth_only;
	} else if (p_use_stencil) {
		if (clear_depth_stencil_state.stencil_only == nil) {
			clear_depth_stencil_state.stencil_only = resource_factory->new_depth_stencil_state(false, true);
		}
		return clear_depth_stencil_state.stencil_only;
	} else {
		if (clear_depth_stencil_state.none == nil) {
			clear_depth_stencil_state.none = resource_factory->new_depth_stencil_state(false, false);
		}
		return clear_depth_stencil_state.none;
	}
}
