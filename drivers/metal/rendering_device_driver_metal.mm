/**************************************************************************/
/*  rendering_device_driver_metal.mm                                      */
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

#include "rendering_device_driver_metal.h"

#include "core/config/project_settings.h"
#include "core/io/compression.h"
#include "core/string/ustring.h"
#include "metal_context.h"
#include "pixel_formats.h"

#include "spirv_msl.hpp"
#include "spirv_parser.hpp"
#include <Metal/MTLTexture.h>
#include <Metal/Metal.h>
#import <compression.h>

/*****************/
/**** GENERIC ****/
/*****************/

// RDD::CompareOperator == VkCompareOp.
static_assert(ENUM_MEMBERS_EQUAL(RDD::COMPARE_OP_NEVER, MTLCompareFunctionNever));
static_assert(ENUM_MEMBERS_EQUAL(RDD::COMPARE_OP_LESS, MTLCompareFunctionLess));
static_assert(ENUM_MEMBERS_EQUAL(RDD::COMPARE_OP_EQUAL, MTLCompareFunctionEqual));
static_assert(ENUM_MEMBERS_EQUAL(RDD::COMPARE_OP_LESS_OR_EQUAL, MTLCompareFunctionLessEqual));
static_assert(ENUM_MEMBERS_EQUAL(RDD::COMPARE_OP_GREATER, MTLCompareFunctionGreater));
static_assert(ENUM_MEMBERS_EQUAL(RDD::COMPARE_OP_NOT_EQUAL, MTLCompareFunctionNotEqual));
static_assert(ENUM_MEMBERS_EQUAL(RDD::COMPARE_OP_GREATER_OR_EQUAL, MTLCompareFunctionGreaterEqual));
static_assert(ENUM_MEMBERS_EQUAL(RDD::COMPARE_OP_ALWAYS, MTLCompareFunctionAlways));

// static_assert(ARRAYS_COMPATIBLE_FIELDWISE(Rect2i, VkRect2D));

_FORCE_INLINE_ MTLSize mipmapLevelSizeFromTexture(id<MTLTexture> tex, NSUInteger level) {
	MTLSize lvlSize;
	lvlSize.width = std::max(tex.width >> level, 1UL);
	lvlSize.height = std::max(tex.height >> level, 1UL);
	lvlSize.depth = std::max(tex.depth >> level, 1UL);
	return lvlSize;
}

_FORCE_INLINE_ MTLSize mipmapLevelSizeFromSize(MTLSize size, NSUInteger level) {
	MTLSize lvlSize;
	lvlSize.width = std::max(size.width >> level, 1UL);
	lvlSize.height = std::max(size.height >> level, 1UL);
	lvlSize.depth = std::max(size.depth >> level, 1UL);
	return lvlSize;
}

_FORCE_INLINE_ static bool operator==(MTLSize a, MTLSize b) {
	return a.width == b.width && a.height == b.height && a.depth == b.depth;
}

_FORCE_INLINE_ static bool operator!=(MTLSize a, MTLSize b) {
	return !(a == b);
}

/****************/
/**** MEMORY ****/
/****************/

/*****************/
/**** BUFFERS ****/
/*****************/

RDD::BufferID RenderingDeviceDriverMetal::buffer_create(uint64_t p_size, BitField<BufferUsageBits> p_usage, MemoryAllocationType p_allocation_type) {
	MTLResourceOptions options = MTLResourceHazardTrackingModeTracked;
	switch (p_allocation_type) {
		case MEMORY_ALLOCATION_TYPE_CPU:
			options = MTLResourceStorageModeShared;
			break;
		case MEMORY_ALLOCATION_TYPE_GPU:
			options = MTLResourceStorageModePrivate;
			break;
	}

	id<MTLBuffer> obj = [device newBufferWithLength:p_size options:options];
	ERR_FAIL_NULL_V_MSG(obj, BufferID(), "Can't create buffer of size: " + itos(p_size));
	return rid::make(obj);
}

bool RenderingDeviceDriverMetal::buffer_set_texel_format(BufferID p_buffer, DataFormat p_format) {
	// TODO(sgc): Is there anything to do here?
	return true;
}

void RenderingDeviceDriverMetal::buffer_free(BufferID p_buffer) {
	rid::release(p_buffer);
}

uint64_t RenderingDeviceDriverMetal::buffer_get_allocation_size(BufferID p_buffer) {
	id<MTLBuffer> obj = rid::get(p_buffer);
	return obj.allocatedSize;
}

uint8_t *RenderingDeviceDriverMetal::buffer_map(BufferID p_buffer) {
	id<MTLBuffer> obj = rid::get(p_buffer);
	ERR_FAIL_COND_V_MSG(obj.storageMode != MTLStorageModeShared, nullptr, "Unable to map private buffers");
	return (uint8_t *)obj.contents;
}

void RenderingDeviceDriverMetal::buffer_unmap(BufferID p_buffer) {
	// nothing to do
}

/*****************/
/**** TEXTURE ****/
/*****************/

#pragma mark - Format Conversions

const MTLTextureType RenderingDeviceDriverMetal::texture_type[RD::TEXTURE_TYPE_MAX] = {
	MTLTextureType1D,
	MTLTextureType2D,
	MTLTextureType3D,
	MTLTextureTypeCube,
	MTLTextureType1DArray,
	MTLTextureType2DArray,
	MTLTextureTypeCubeArray,
};

RDD::TextureID RenderingDeviceDriverMetal::texture_create(const TextureFormat &p_format, const TextureView &p_view) {
	MTLTextureDescriptor *desc = [MTLTextureDescriptor new];
	desc.textureType = texture_type[p_format.texture_type];

	PixelFormats &formats = context->get_pixel_formats();
	desc.pixelFormat = formats.getMTLPixelFormat(p_format.format);
	MVKMTLFmtCaps format_caps = formats.getCapabilities(desc.pixelFormat);

	desc.width = p_format.width;
	desc.height = p_format.height;
	desc.depth = p_format.depth;
	desc.mipmapLevelCount = p_format.mipmaps;

	if (p_format.texture_type == TEXTURE_TYPE_1D_ARRAY ||
			p_format.texture_type == TEXTURE_TYPE_2D_ARRAY) {
		desc.arrayLength = p_format.array_layers;
	} else if (p_format.texture_type == TEXTURE_TYPE_CUBE_ARRAY) {
		desc.arrayLength = p_format.array_layers / 6;
	}

	if (p_format.samples > TEXTURE_SAMPLES_1) {
		SampleCount supported = context->get_device_properties().find_nearest_supported_sample_count(p_format.samples);

		if (supported > SampleCount1) {
			bool ok = p_format.texture_type == TEXTURE_TYPE_2D || p_format.texture_type == TEXTURE_TYPE_2D_ARRAY;
			if (ok) {
				switch (p_format.texture_type) {
					case TEXTURE_TYPE_2D:
						desc.textureType = MTLTextureType2DMultisample;
						break;
					case TEXTURE_TYPE_2D_ARRAY:
						desc.textureType = MTLTextureType2DMultisampleArray;
						break;
					default:
						break;
				}
				desc.sampleCount = (NSUInteger)supported;
				if (p_format.mipmaps > 1) {
					// For a buffer-backed or multisample textures, the value must be 1.
					WARN_PRINT("mipmaps == 1 for multi-sample textures");
					desc.mipmapLevelCount = 1;
				}
			} else {
				WARN_PRINT("Unsupported multi-sample texture type; disabling multi-sample");
			}
		}
	}

	static const MTLTextureSwizzle component_swizzle[TEXTURE_SWIZZLE_MAX] = {
		static_cast<MTLTextureSwizzle>(255), // IDENTITY
		MTLTextureSwizzleZero,
		MTLTextureSwizzleOne,
		MTLTextureSwizzleRed,
		MTLTextureSwizzleGreen,
		MTLTextureSwizzleBlue,
		MTLTextureSwizzleAlpha,
	};

	MTLTextureSwizzleChannels swizzle = MTLTextureSwizzleChannelsMake(
			p_view.swizzle_r != TEXTURE_SWIZZLE_IDENTITY ? component_swizzle[p_view.swizzle_r] : MTLTextureSwizzleRed,
			p_view.swizzle_g != TEXTURE_SWIZZLE_IDENTITY ? component_swizzle[p_view.swizzle_g] : MTLTextureSwizzleGreen,
			p_view.swizzle_b != TEXTURE_SWIZZLE_IDENTITY ? component_swizzle[p_view.swizzle_b] : MTLTextureSwizzleBlue,
			p_view.swizzle_a != TEXTURE_SWIZZLE_IDENTITY ? component_swizzle[p_view.swizzle_a] : MTLTextureSwizzleAlpha);

	static MTLTextureSwizzleChannels IDENTITY_SWIZZLE = {
		.red = MTLTextureSwizzleRed,
		.green = MTLTextureSwizzleGreen,
		.blue = MTLTextureSwizzleBlue,
		.alpha = MTLTextureSwizzleAlpha,
	};

	bool no_swizzle = memcmp(&IDENTITY_SWIZZLE, &swizzle, sizeof(MTLTextureSwizzleChannels)) == 0;

	if (!no_swizzle) {
		desc.swizzle = swizzle;
	}

	// Usage.
	MTLResourceOptions options = MTLResourceCPUCacheModeDefaultCache | MTLResourceHazardTrackingModeTracked;
	if (p_format.usage_bits & TEXTURE_USAGE_CPU_READ_BIT) {
		options |= MTLResourceStorageModeManaged;
	} else {
		options |= MTLResourceStorageModePrivate;
	}
	desc.resourceOptions = options;

	if (p_format.usage_bits & TEXTURE_USAGE_SAMPLING_BIT) {
		desc.usage |= MTLTextureUsageShaderRead;
	}

	if (p_format.usage_bits & TEXTURE_USAGE_STORAGE_BIT) {
		desc.usage |= MTLTextureUsageShaderWrite;
	}

	if (p_format.usage_bits & TEXTURE_USAGE_STORAGE_ATOMIC_BIT) {
	}

	bool can_be_attachment = mvkIsAnyFlagEnabled(format_caps, (kMVKMTLFmtCapsColorAtt | kMVKMTLFmtCapsDSAtt));

	if (mvkIsAnyFlagEnabled(p_format.usage_bits, TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) &&
			can_be_attachment) {
		desc.usage |= MTLTextureUsageRenderTarget;
	}

	if (p_format.usage_bits & TEXTURE_USAGE_INPUT_ATTACHMENT_BIT) {
		desc.usage |= MTLTextureUsageShaderRead;
	}

	if (p_format.usage_bits & TEXTURE_USAGE_VRS_ATTACHMENT_BIT) {
		BOOL ok = [device supportsRasterizationRateMapWithLayerCount:1];
		ERR_FAIL_V_MSG(RDD::TextureID(), "unsupported: TEXTURE_USAGE_VRS_ATTACHMENT_BIT");
	}

	if (mvkIsAnyFlagEnabled(p_format.usage_bits, TEXTURE_USAGE_CAN_UPDATE_BIT | TEXTURE_USAGE_CAN_COPY_TO_BIT) &&
			can_be_attachment && no_swizzle) {
		// per MoltenVK, can be cleared as a render attachment
		desc.usage |= MTLTextureUsageRenderTarget;
	}
	if (p_format.usage_bits & TEXTURE_USAGE_CAN_COPY_FROM_BIT) {
		// covered by blits
	}

	// create texture views with a different component layout
	desc.usage |= MTLTextureUsagePixelFormatView;

	// Allocate memory.

	uint32_t width, height;
	uint32_t image_size = get_image_format_required_size(p_format.format, p_format.width, p_format.height, p_format.depth, p_format.mipmaps, &width, &height);

	// check if it is a linear format for atomics
	bool needs_buffer = ((p_format.usage_bits & TEXTURE_USAGE_CPU_READ_BIT) == TEXTURE_USAGE_CPU_READ_BIT) ||
			(p_format.array_layers == 1 && p_format.mipmaps == 1 && p_format.texture_type == TEXTURE_TYPE_2D && p_format.usage_bits & TEXTURE_USAGE_STORAGE_BIT &&
					(p_format.format == DATA_FORMAT_R32_UINT || p_format.format == DATA_FORMAT_R32_SINT));

	id<MTLTexture> obj = nil;
	if (needs_buffer) {
		// this logic was extracted from MoltenVK

		size_t row_alignment = context->get_texel_buffer_alignment_for_format(p_format.format);
		size_t byte_count = 0;
		MTLSize size = MTLSizeMake(p_format.width, p_format.height, p_format.depth);
		MTLPixelFormat pixel_format = desc.pixelFormat;
		for (uint32_t mipLvl = 0; mipLvl < p_format.mipmaps; mipLvl++) {
			MTLSize mipExtent = mipmapLevelSizeFromSize(size, mipLvl);
			size_t bytes_per_row = formats.getBytesPerRow(pixel_format, mipExtent.width);
			bytes_per_row = mvkAlignByteCount(bytes_per_row, row_alignment);
			size_t bytes_per_layer = formats.getBytesPerLayer(pixel_format, bytes_per_row, mipExtent.height);
			byte_count += bytes_per_layer * mipExtent.depth * p_format.array_layers;
		}

		size_t bytes_per_row = formats.getBytesPerRow(pixel_format, size.width);
		bytes_per_row = mvkAlignByteCount(bytes_per_row, row_alignment);

		id<MTLBuffer> buf = [device newBufferWithLength:byte_count options:options];
		obj = [buf newTextureWithDescriptor:desc offset:0 bytesPerRow:bytes_per_row];
	} else {
		obj = [device newTextureWithDescriptor:desc];
	}
	ERR_FAIL_NULL_V_MSG(obj, TextureID(), "Unable to create texture.");

	return rid::make(obj);
}

RDD::TextureID RenderingDeviceDriverMetal::texture_create_from_extension(uint64_t p_native_texture, TextureType p_type, DataFormat p_format, uint32_t p_array_layers, bool p_depth_stencil) {
	ERR_FAIL_V_MSG(RDD::TextureID(), "not implemented");
}

RDD::TextureID RenderingDeviceDriverMetal::texture_create_shared(TextureID p_original_texture, const TextureView &p_view) {
	id<MTLTexture> src_texture = rid::get(p_original_texture);

	if (src_texture.sampleCount > 1) {
		WARN_PRINT("Is it safe to create a shared texture from multi-sample texture?");
	}

	PixelFormats &pf = context->get_pixel_formats();

	MTLPixelFormat format = pf.getMTLPixelFormat(p_view.format);

	static const MTLTextureSwizzle component_swizzle[TEXTURE_SWIZZLE_MAX] = {
		static_cast<MTLTextureSwizzle>(255), // IDENTITY
		MTLTextureSwizzleZero,
		MTLTextureSwizzleOne,
		MTLTextureSwizzleRed,
		MTLTextureSwizzleGreen,
		MTLTextureSwizzleBlue,
		MTLTextureSwizzleAlpha,
	};

#define SWIZZLE(C, CHAN) (p_view.swizzle_##C != TEXTURE_SWIZZLE_IDENTITY ? component_swizzle[p_view.swizzle_##C] : MTLTextureSwizzle##CHAN)
	MTLTextureSwizzleChannels swizzle = MTLTextureSwizzleChannelsMake(
			SWIZZLE(r, Red),
			SWIZZLE(g, Green),
			SWIZZLE(b, Blue),
			SWIZZLE(a, Alpha));
#undef SWIZZLE
	id<MTLTexture> obj = [src_texture newTextureViewWithPixelFormat:format
														textureType:src_texture.textureType
															 levels:NSMakeRange(0, src_texture.mipmapLevelCount)
															 slices:NSMakeRange(0, src_texture.arrayLength)
															swizzle:swizzle];
	ERR_FAIL_NULL_V_MSG(obj, TextureID(), "Unable to create shared texture");
	return rid::make(obj);
}

RDD::TextureID RenderingDeviceDriverMetal::texture_create_shared_from_slice(TextureID p_original_texture, const TextureView &p_view, TextureSliceType p_slice_type, uint32_t p_layer, uint32_t p_layers, uint32_t p_mipmap, uint32_t p_mipmaps) {
	id<MTLTexture> src_texture = rid::get(p_original_texture);

	PixelFormats &pf = context->get_pixel_formats();
	static const MTLTextureType view_types[] = {
		MTLTextureType1D, // TEXTURE_TYPE_1D
		MTLTextureType1D, // TEXTURE_TYPE_1D_ARRAY
		MTLTextureType2D, // TEXTURE_TYPE_2D
		MTLTextureType2D, // TEXTURE_TYPE_2D_ARRAY
		MTLTextureType2D, // MTLTextureType2DMultisample
		MTLTextureType2D, // TEXTURE_TYPE_CUBE
		MTLTextureType2D, // TEXTURE_TYPE_CUBE_ARRAY
		MTLTextureType2D, // TEXTURE_TYPE_3D
		MTLTextureType2D, // MTLTextureType2DMultisampleArray
	};

	MTLTextureType textureType = view_types[src_texture.textureType];
	switch (p_slice_type) {
		case TEXTURE_SLICE_2D: {
			textureType = MTLTextureType2D;
		} break;
		case TEXTURE_SLICE_3D: {
			textureType = MTLTextureType3D;
		} break;
		case TEXTURE_SLICE_CUBEMAP: {
			textureType = MTLTextureTypeCube;
		} break;
		case TEXTURE_SLICE_2D_ARRAY: {
			textureType = MTLTextureType2DArray;
		} break;
	}

	MTLPixelFormat format = pf.getMTLPixelFormat(p_view.format);

	static const MTLTextureSwizzle component_swizzle[TEXTURE_SWIZZLE_MAX] = {
		static_cast<MTLTextureSwizzle>(255), // IDENTITY
		MTLTextureSwizzleZero,
		MTLTextureSwizzleOne,
		MTLTextureSwizzleRed,
		MTLTextureSwizzleGreen,
		MTLTextureSwizzleBlue,
		MTLTextureSwizzleAlpha,
	};

#define SWIZZLE(C, CHAN) (p_view.swizzle_##C != TEXTURE_SWIZZLE_IDENTITY ? component_swizzle[p_view.swizzle_##C] : MTLTextureSwizzle##CHAN)
	MTLTextureSwizzleChannels swizzle = MTLTextureSwizzleChannelsMake(
			SWIZZLE(r, Red),
			SWIZZLE(g, Green),
			SWIZZLE(b, Blue),
			SWIZZLE(a, Alpha));
#undef SWIZZLE
	id<MTLTexture> obj = [src_texture newTextureViewWithPixelFormat:format
														textureType:textureType
															 levels:NSMakeRange(p_mipmap, p_mipmaps)
															 slices:NSMakeRange(p_layer, p_layers)
															swizzle:swizzle];
	ERR_FAIL_NULL_V_MSG(obj, TextureID(), "Unable to create shared texture");
	return rid::make(obj);
}

void RenderingDeviceDriverMetal::texture_free(TextureID p_texture) {
	rid::release(p_texture);
}

uint64_t RenderingDeviceDriverMetal::texture_get_allocation_size(TextureID p_texture) {
	id<MTLTexture> obj = rid::get(p_texture);
	return obj.allocatedSize;
}

void RenderingDeviceDriverMetal::texture_get_copyable_layout(TextureID p_texture, const TextureSubresource &p_subresource, TextureCopyableLayout *r_layout) {
	id<MTLTexture> obj = rid::get(p_texture);
	*r_layout = {};

	// Tight.
	uint32_t w = obj.width;
	uint32_t h = obj.height;
	uint32_t d = obj.depth;

	PixelFormats &pf = context->get_pixel_formats();
	DataFormat format = pf.getDataFormat(obj.pixelFormat);
	if (p_subresource.mipmap > 0) {
		r_layout->offset = get_image_format_required_size(format, w, h, d, p_subresource.mipmap - 1);
	} else {
		r_layout->offset = 0;
	}

	for (uint32_t i = 0; i < p_subresource.mipmap; i++) {
		w = MAX(1u, w >> 1);
		h = MAX(1u, h >> 1);
		d = MAX(1u, d >> 1);
	}

	r_layout->size = get_image_format_required_size(format, w, h, d, 1);
	r_layout->row_pitch = r_layout->size / (h * d);
	r_layout->depth_pitch = r_layout->size / d;
	r_layout->layer_pitch = r_layout->size / obj.arrayLength;
}

uint32_t RenderingDeviceDriverMetal::_compute_plane_slice(DataFormat p_format, BitField<TextureAspectBits> p_aspect_bits) {
	TextureAspect aspect = TEXTURE_ASPECT_MAX;

	if (p_aspect_bits.has_flag(TEXTURE_ASPECT_COLOR_BIT)) {
		DEV_ASSERT(aspect == TEXTURE_ASPECT_MAX);
		aspect = TEXTURE_ASPECT_COLOR;
	}
	if (p_aspect_bits.has_flag(TEXTURE_ASPECT_DEPTH_BIT)) {
		DEV_ASSERT(aspect == TEXTURE_ASPECT_MAX);
		aspect = TEXTURE_ASPECT_DEPTH;
	}
	if (p_aspect_bits.has_flag(TEXTURE_ASPECT_STENCIL_BIT)) {
		DEV_ASSERT(aspect == TEXTURE_ASPECT_MAX);
		aspect = TEXTURE_ASPECT_STENCIL;
	}

	DEV_ASSERT(aspect != TEXTURE_ASPECT_MAX);

	return _compute_plane_slice(p_format, aspect);
}

uint32_t RenderingDeviceDriverMetal::_compute_plane_slice(DataFormat p_format, TextureAspect p_aspect) {
	switch (p_aspect) {
		case TEXTURE_ASPECT_COLOR:
			// The plane must be 0 for the color aspect (assuming the format is a regular color one, which must be the case).
			return 0;
		case TEXTURE_ASPECT_DEPTH:
			// The plane must be 0 for the color or depth aspect
			return 0;
		case TEXTURE_ASPECT_STENCIL:
			// The plane may be 0 for the stencil aspect (if the format is stencil-only), or 1 (if the format is depth-stencil; other cases are ill).
			return format_get_plane_count(p_format) == 2 ? 1 : 0;
		default:
			DEV_ASSERT(false);
			return 0;
	}
}

uint8_t *RenderingDeviceDriverMetal::texture_map(TextureID p_texture, const TextureSubresource &p_subresource) {
	id<MTLTexture> obj = rid::get(p_texture);
	ERR_FAIL_NULL_V_MSG(obj.buffer, nullptr, "texture is not created from a buffer");

	PixelFormats &pf = context->get_pixel_formats();
	DataFormat format = pf.getDataFormat(obj.pixelFormat);
	uint32_t plane = _compute_plane_slice(format, p_subresource.aspect);

	// TODO(sgc): calculate offset into buffer for the requested subresource
	if (p_subresource.layer > 1 || p_subresource.mipmap > 0) {
		CRASH_NOW_MSG("implement subresource mapping");
	}

	return (uint8_t *)obj.buffer.contents;
}

void RenderingDeviceDriverMetal::texture_unmap(TextureID p_texture) {
	// nothing to do
}

BitField<RDD::TextureUsageBits> RenderingDeviceDriverMetal::texture_get_usages_supported_by_format(DataFormat p_format, bool p_cpu_readable) {
	PixelFormats &pf = context->get_pixel_formats();
	MTLPixelFormat format = pf.getMTLPixelFormat(p_format);

	// Everything supported by default if the format is valid.
	// TODO(sgc): implement correct supported flags
	return format == MTLPixelFormatInvalid ? 0 : INT64_MAX;
}

/*****************/
/**** SAMPLER ****/
/*****************/

const MTLCompareFunction RenderingDeviceDriverMetal::compare_operators[RD::COMPARE_OP_MAX] = {
	MTLCompareFunctionNever,
	MTLCompareFunctionLess,
	MTLCompareFunctionEqual,
	MTLCompareFunctionLessEqual,
	MTLCompareFunctionGreater,
	MTLCompareFunctionNotEqual,
	MTLCompareFunctionGreaterEqual,
	MTLCompareFunctionAlways,
};

const MTLStencilOperation RenderingDeviceDriverMetal::stencil_operations[RD::STENCIL_OP_MAX] = {
	MTLStencilOperationKeep,
	MTLStencilOperationZero,
	MTLStencilOperationReplace,
	MTLStencilOperationIncrementClamp,
	MTLStencilOperationDecrementClamp,
	MTLStencilOperationInvert,
	MTLStencilOperationIncrementWrap,
	MTLStencilOperationDecrementWrap,
};

const MTLBlendFactor RenderingDeviceDriverMetal::blend_factors[RD::BLEND_FACTOR_MAX] = {
	MTLBlendFactorZero,
	MTLBlendFactorOne,
	MTLBlendFactorSourceColor,
	MTLBlendFactorOneMinusSourceColor,
	MTLBlendFactorDestinationColor,
	MTLBlendFactorOneMinusDestinationColor,
	MTLBlendFactorSourceAlpha,
	MTLBlendFactorOneMinusSourceAlpha,
	MTLBlendFactorDestinationAlpha,
	MTLBlendFactorOneMinusDestinationAlpha,
	MTLBlendFactorBlendColor,
	MTLBlendFactorOneMinusBlendColor,
	MTLBlendFactorBlendAlpha,
	MTLBlendFactorOneMinusBlendAlpha,
	MTLBlendFactorSourceAlphaSaturated,
	MTLBlendFactorSource1Color,
	MTLBlendFactorOneMinusSource1Color,
	MTLBlendFactorSource1Alpha,
	MTLBlendFactorOneMinusSource1Alpha,
};
const MTLBlendOperation RenderingDeviceDriverMetal::blend_operations[RD::BLEND_OP_MAX] = {
	MTLBlendOperationAdd,
	MTLBlendOperationSubtract,
	MTLBlendOperationReverseSubtract,
	MTLBlendOperationMin,
	MTLBlendOperationMax,
};

const MTLSamplerAddressMode RenderingDeviceDriverMetal::address_modes[RD::SAMPLER_REPEAT_MODE_MAX] = {
	MTLSamplerAddressModeRepeat,
	MTLSamplerAddressModeMirrorRepeat,
	MTLSamplerAddressModeClampToEdge,
	MTLSamplerAddressModeClampToBorderColor,
	MTLSamplerAddressModeMirrorClampToEdge,
};

const MTLSamplerBorderColor RenderingDeviceDriverMetal::sampler_border_colors[RD::SAMPLER_BORDER_COLOR_MAX] = {
	MTLSamplerBorderColorTransparentBlack,
	MTLSamplerBorderColorTransparentBlack,
	MTLSamplerBorderColorOpaqueBlack,
	MTLSamplerBorderColorOpaqueBlack,
	MTLSamplerBorderColorOpaqueWhite,
	MTLSamplerBorderColorOpaqueWhite,
};

RDD::SamplerID RenderingDeviceDriverMetal::sampler_create(const SamplerState &p_state) {
	MTLSamplerDescriptor *desc = [MTLSamplerDescriptor new];
	desc.supportArgumentBuffers = YES;

	desc.magFilter = p_state.mag_filter == SAMPLER_FILTER_LINEAR ? MTLSamplerMinMagFilterLinear : MTLSamplerMinMagFilterNearest;
	desc.minFilter = p_state.min_filter == SAMPLER_FILTER_LINEAR ? MTLSamplerMinMagFilterLinear : MTLSamplerMinMagFilterNearest;
	desc.mipFilter = p_state.mip_filter == SAMPLER_FILTER_LINEAR ? MTLSamplerMipFilterLinear : MTLSamplerMipFilterNearest;

	desc.sAddressMode = address_modes[p_state.repeat_u];
	desc.tAddressMode = address_modes[p_state.repeat_v];
	desc.rAddressMode = address_modes[p_state.repeat_w];

	if (p_state.use_anisotropy) {
		desc.maxAnisotropy = p_state.anisotropy_max;
	}

	desc.compareFunction = compare_operators[p_state.compare_op];

	desc.lodMinClamp = p_state.min_lod;
	desc.lodMaxClamp = p_state.max_lod;

	desc.borderColor = sampler_border_colors[p_state.border_color];

	desc.normalizedCoordinates = !p_state.unnormalized_uvw;

	if (p_state.lod_bias != 0.0) {
		WARN_PRINT_ONCE("Metal does not support LOD bias for samplers.");
	}

	id<MTLSamplerState> obj = [device newSamplerStateWithDescriptor:desc];
	ERR_FAIL_NULL_V_MSG(obj, SamplerID(), "newSamplerStateWithDescriptor failed");
	return rid::make(obj);
}

void RenderingDeviceDriverMetal::sampler_free(SamplerID p_sampler) {
	rid::release(p_sampler);
}

bool RenderingDeviceDriverMetal::sampler_is_format_supported_for_filter(DataFormat p_format, SamplerFilter p_filter) {
	// Return true for everything until we dig into feature sets
	return true;
}

/**********************/
/**** VERTEX ARRAY ****/
/**********************/

RDD::VertexFormatID RenderingDeviceDriverMetal::vertex_format_create(VectorView<VertexAttribute> p_vertex_attribs) {
	MTLVertexDescriptor *desc = MTLVertexDescriptor.vertexDescriptor;

	PixelFormats &pixel_formats = context->get_pixel_formats();

	for (int i = 0; i < p_vertex_attribs.size(); i++) {
		VertexAttribute const &vf = p_vertex_attribs[i];

		ERR_FAIL_COND_V_MSG(get_format_vertex_size(vf.format) == 0, VertexFormatID(),
				"Data format for attachment (" + itos(i) + "), '" + FORMAT_NAMES[vf.format] + "', is not valid for a vertex array.");

		desc.attributes[vf.location].format = pixel_formats.getMTLVertexFormat(vf.format);
		desc.attributes[vf.location].offset = vf.offset;
		uint32_t idx = context->get_metal_buffer_index_for_vertex_attribute_binding(i);
		desc.attributes[vf.location].bufferIndex = idx;
		if (vf.stride == 0) {
			desc.layouts[idx].stepFunction = MTLVertexStepFunctionPerVertex;
			desc.layouts[idx].stepRate = 1;
			desc.layouts[idx].stride = pixel_formats.getBytesPerBlock(vf.format);
		} else {
			desc.layouts[idx].stepFunction = vf.frequency == VERTEX_FREQUENCY_VERTEX ? MTLVertexStepFunctionPerVertex : MTLVertexStepFunctionPerInstance;
			desc.layouts[idx].stepRate = 1;
			desc.layouts[idx].stride = vf.stride;
		}
	}

	return rid::make(desc);
}

void RenderingDeviceDriverMetal::vertex_format_free(VertexFormatID p_vertex_format) {
	rid::release(p_vertex_format);
}

/******************/
/**** BARRIERS ****/
/******************/

void RenderingDeviceDriverMetal::command_pipeline_barrier(
		CommandBufferID p_cmd_buffer,
		BitField<PipelineStageBits> p_src_stages,
		BitField<PipelineStageBits> p_dst_stages,
		VectorView<MemoryBarrier> p_memory_barriers,
		VectorView<BufferBarrier> p_buffer_barriers,
		VectorView<TextureBarrier> p_texture_barriers) {
	WARN_PRINT_ONCE("not implemented");
}

/*************************/
/**** COMMAND BUFFERS ****/
/*************************/

// ----- POOL -----

RDD::CommandPoolID RenderingDeviceDriverMetal::command_pool_create(CommandBufferType p_cmd_buffer_type) {
	DEV_ASSERT(p_cmd_buffer_type == COMMAND_BUFFER_TYPE_PRIMARY);
	id<MTLCommandQueue> queue = context->get_graphics_queue();
	return rid::make(queue);
}

void RenderingDeviceDriverMetal::command_pool_free(CommandPoolID p_cmd_pool) {
	rid::release(p_cmd_pool);
}

// ----- BUFFER -----

RDD::CommandBufferID RenderingDeviceDriverMetal::command_buffer_create(CommandBufferType p_cmd_buffer_type, CommandPoolID p_cmd_pool) {
	id<MTLCommandQueue> queue = rid::get(p_cmd_pool);
	MDCommandBuffer *obj = new MDCommandBuffer(queue, context);
	command_buffers.push_back(obj);
	return CommandBufferID(obj);
}

bool RenderingDeviceDriverMetal::command_buffer_begin(CommandBufferID p_cmd_buffer) {
	MDCommandBuffer *obj = (MDCommandBuffer *)(p_cmd_buffer.id);
	obj->begin();
	return true;
}

bool RenderingDeviceDriverMetal::command_buffer_begin_secondary(CommandBufferID p_cmd_buffer, RenderPassID p_render_pass, uint32_t p_subpass, FramebufferID p_framebuffer) {
	ERR_FAIL_V_MSG(false, "not implemented");
}

void RenderingDeviceDriverMetal::command_buffer_end(CommandBufferID p_cmd_buffer) {
	MDCommandBuffer *obj = (MDCommandBuffer *)(p_cmd_buffer.id);
	obj->end();
}

void RenderingDeviceDriverMetal::command_buffer_execute_secondary(CommandBufferID p_cmd_buffer, VectorView<CommandBufferID> p_secondary_cmd_buffers) {
	ERR_FAIL_MSG("not implemented");
}

/*********************/
/**** FRAMEBUFFER ****/
/*********************/

RDD::FramebufferID RenderingDeviceDriverMetal::framebuffer_create(RenderPassID p_render_pass, VectorView<TextureID> p_attachments, uint32_t p_width, uint32_t p_height) {
	MDRenderPass *pass = (MDRenderPass *)(p_render_pass.id);

	LocalVector<MTL::Texture> textures;
	textures.resize(p_attachments.size());

	for (int i = 0; i < p_attachments.size(); i += 1) {
		MDAttachment const &a = pass->attachments[i];
		id<MTLTexture> tex = rid::get(p_attachments[i]);
		if (tex == nil) {
			WARN_PRINT("Invalid texture for attachment " + itos(i));
		}
		if (a.samples > 1) {
			if (tex.sampleCount != a.samples) {
				WARN_PRINT("Mismatched sample count for attachment " + itos(i) + "; expected " + itos(a.samples) + ", got " + itos(tex.sampleCount));
			}
		}
		textures[i] = tex;
	}

	MDFrameBuffer *fb = new MDFrameBuffer(textures, Size2i(p_width, p_height));
	return FramebufferID(fb);
}

void RenderingDeviceDriverMetal::framebuffer_free(FramebufferID p_framebuffer) {
	MDFrameBuffer *obj = (MDFrameBuffer *)(p_framebuffer.id);
	delete obj;
}

/****************/
/**** SHADER ****/
/****************/

#define SHADER_BINARY_VERSION 1

String RenderingDeviceDriverMetal::shader_get_binary_cache_key() {
	return "Metal-SV" + uitos(SHADER_BINARY_VERSION);
}

#include "cista.h"

namespace data = cista::offset;
using RDM = RenderingDeviceDriverMetal;

struct ComputeSize {
	uint32_t x;
	uint32_t y;
	uint32_t z;
};

struct ShaderStageData {
	RD::ShaderStage stage;
	data::string entry_point_name;
	data::vector<uint8_t> source_data;
	uint32_t source_size;
};

struct SpecializationConstantData {
	uint32_t constant_id;
	RD::PipelineSpecializationConstantType type;
	ShaderStageUsage stages;
	// used_stages specifies the stages the constant is used by Metal
	ShaderStageUsage used_stages;
	uint32_t int_value;
};

struct UniformData {
	RD::UniformType type;
	uint32_t binding;
	bool writable;
	RDM::LengthType length_type;
	uint32_t length;
	ShaderStageUsage stages;
	// active_stages specifies the stages the uniform data is
	// used by the Metal shader
	ShaderStageUsage active_stages;
	data::hash_map<RD::ShaderStage, BindingInfo> bindings;
	data::hash_map<RD::ShaderStage, BindingInfo> bindings_secondary;
};

struct UniformSetData {
	uint32_t index;
	data::vector<UniformData> uniforms;
};

struct PushConstantData {
	uint32_t size;
	ShaderStageUsage stages;
	ShaderStageUsage used_stages;
	data::hash_map<RD::ShaderStage, uint32_t> msl_binding;
};

struct ShaderBinaryData {
	data::string shader_name;
	uint32_t vertex_input_mask;
	uint32_t fragment_output_mask;
	uint32_t spirv_specialization_constants_ids_mask;
	uint32_t is_compute;
	ComputeSize compute_local_size;
	PushConstantData push_constant;
	data::vector<ShaderStageData> stages;
	data::vector<SpecializationConstantData> constants;
	data::vector<UniformSetData> uniforms;
};

constexpr auto const MODE = cista::mode::WITH_VERSION;

const char *shader_stage_names[RenderingDevice::SHADER_STAGE_MAX] = {
	"Vertex",
	"Fragment",
	"TesselationControl",
	"TesselationEvaluation",
	"Compute",
};

const uint32_t R32UI_ALIGNMENT_CONSTANT_ID = 65535;

Vector<uint8_t> RenderingDeviceDriverMetal::shader_compile_binary_from_spirv(VectorView<ShaderStageSPIRVData> p_spirv, const String &p_shader_name) {
	using Result = Vector<uint8_t>;

	ShaderReflection spirv_data;
	if (_reflect_spirv(p_spirv, spirv_data) != OK) {
		return {};
	}

	ShaderBinaryData bin_data{};
	if (!p_shader_name.is_empty()) {
		bin_data.shader_name = data::string(p_shader_name.utf8().ptr());
	} else {
		bin_data.shader_name = data::string("unnamed");
	}

	bin_data.vertex_input_mask = spirv_data.vertex_input_mask;
	bin_data.fragment_output_mask = spirv_data.fragment_output_mask;
	bin_data.compute_local_size = ComputeSize{
		.x = spirv_data.compute_local_size[0],
		.y = spirv_data.compute_local_size[1],
		.z = spirv_data.compute_local_size[2],
	};
	bin_data.is_compute = spirv_data.is_compute;
	bin_data.push_constant.size = spirv_data.push_constant_size;
	bin_data.push_constant.stages = (ShaderStageUsage)(uint8_t)spirv_data.push_constant_stages;

	for (uint32_t i = 0; i < spirv_data.uniform_sets.size(); i++) {
		const Vector<ShaderUniform> &spirv_set = spirv_data.uniform_sets[i];
		UniformSetData set{ .index = i };
		for (const ShaderUniform &spirv_uniform : spirv_set) {
			UniformData binding{};
			binding.type = spirv_uniform.type;
			binding.binding = spirv_uniform.binding;
			binding.writable = spirv_uniform.writable;
			binding.stages = (ShaderStageUsage)(uint8_t)spirv_uniform.stages;
			binding.length_type = length_type(spirv_uniform.type);
			binding.length = spirv_uniform.length;
			set.uniforms.push_back(binding);
		}
		bin_data.uniforms.push_back(set);
	}

	for (const ShaderSpecializationConstant &spirv_sc : spirv_data.specialization_constants) {
		SpecializationConstantData spec_constant{};
		spec_constant.type = spirv_sc.type;
		spec_constant.constant_id = spirv_sc.constant_id;
		spec_constant.int_value = spirv_sc.int_value;
		spec_constant.stages = (ShaderStageUsage)(uint8_t)spirv_sc.stages;
		bin_data.constants.push_back(spec_constant);
		bin_data.spirv_specialization_constants_ids_mask |= (1 << spirv_sc.constant_id);
	}

	// Reflection using SPIRV-Cross:
	// https://github.com/KhronosGroup/SPIRV-Cross/wiki/Reflection-API-user-guide

	using spirv_cross::CompilerMSL;

	spirv_cross::CompilerMSL::Options msl_options{};
	msl_options.set_msl_version(context->get_version_major(), context->get_version_minor());
	msl_options.argument_buffers = true;
	msl_options.force_active_argument_buffer_resources = true; // same as MoltenVK when using argument buffers
	msl_options.pad_argument_buffer_resources = true; // same as MoltenVK when using argument buffers
	msl_options.texture_buffer_native = true; // texture_buffer support
	msl_options.use_framebuffer_fetch_subpasses = true;
	msl_options.pad_fragment_output_components = true;
	msl_options.r32ui_alignment_constant_id = R32UI_ALIGNMENT_CONSTANT_ID;

	spirv_cross::CompilerGLSL::Options options{};
	options.vertex.flip_vert_y = true;
#if DEV_ENABLED
	options.emit_line_directives = true;
#endif

	for (int i = 0; i < p_spirv.size(); i++) {
		ShaderStageSPIRVData v = p_spirv[i];
		ShaderStage stage = v.shader_stage;
		char const *stage_name = shader_stage_names[stage];
		uint32_t const *const ir = reinterpret_cast<uint32_t const *const>(v.spirv.ptr());
		size_t word_count = v.spirv.size() / sizeof(uint32_t);
		spirv_cross::Parser parser(ir, word_count);
		try {
			parser.parse();
		} catch (spirv_cross::CompilerError &e) {
			ERR_FAIL_V_MSG(Result(), "Failed to parse IR at stage " + String(shader_stage_names[stage]) + ": " + e.what());
		}

		CompilerMSL compiler(std::move(parser.get_parsed_ir()));
		compiler.set_msl_options(msl_options);
		compiler.set_common_options(options);

		auto active = compiler.get_active_interface_variables();
		auto resources = compiler.get_shader_resources();

		std::string source = compiler.compile();

		ERR_FAIL_COND_V_MSG(compiler.get_entry_points_and_stages().size() != 1, Result(), "Expected a single entry point and stage.");

		auto entry_point_stage = compiler.get_entry_points_and_stages().front();
		auto entry_point = compiler.get_entry_point(entry_point_stage.name, entry_point_stage.execution_model);

		// process specialization constants
		if (!compiler.get_specialization_constants().empty()) {
			for (auto &constant : compiler.get_specialization_constants()) {
				auto res = std::find_if(bin_data.constants.begin(), bin_data.constants.end(), [constant](auto &v) { return v.constant_id == constant.constant_id; });
				if (res) {
					res->used_stages |= 1 << stage;
				} else {
					WARN_PRINT(String(stage_name) + ": unable to find constant_id: " + itos(constant.constant_id));
				}
			}
		}

		// process bindings

		auto &uniform_sets = bin_data.uniforms;
		using BT = spirv_cross::SPIRType::BaseType;

		// get_decoration returns a std::optional containing the value of the
		// decoration, if it exists.
		auto get_decoration = [&compiler](spirv_cross::ID id, spv::Decoration decoration) {
			std::optional<uint32_t> res;
			if (compiler.has_decoration(id, decoration)) {
				res = compiler.get_decoration(id, decoration);
			}
			return res;
		};

		auto descriptor_bindings = [&compiler, &active, &uniform_sets, stage, &get_decoration](char const *resource, auto &resources) {
			for (auto &res : resources) {
				auto dset = get_decoration(res.id, spv::DecorationDescriptorSet);
				auto dbin = get_decoration(res.id, spv::DecorationBinding);
				auto name = compiler.get_name(res.id);
				UniformData *found = nullptr;
				if (dset.has_value() && dbin.has_value() && dset.value() < uniform_sets.size()) {
					auto &set = uniform_sets[dset.value()];
					found = std::find_if(set.uniforms.begin(), set.uniforms.end(), [dbin](auto &v) { return dbin == v.binding; });
				}

				ERR_FAIL_NULL_V_MSG(found, ERR_CANT_CREATE, "UniformData not found");

				bool is_active = active.find(res.id) != active.end();
				if (is_active) {
					found->active_stages |= 1 << stage;
				}

				BindingInfo primary{};

				auto a_base_type = compiler.get_type(res.base_type_id);
				auto basetype = a_base_type.basetype;

				size_t struct_size = 0;
				// potentially contains the MTLBindingAccess after examining
				// metadata about the basetype
				std::optional<MTLBindingAccess> opt_access;
				std::optional<uint32_t> primary_binding;

				switch (basetype) {
					case BT::Struct: {
						primary.dataType = MTLDataTypePointer;
						struct_size = compiler.get_declared_struct_size(a_base_type);
						auto flags = compiler.get_buffer_block_flags(res.id);
						if (!flags.get(spv::DecorationNonWritable)) {
							if (!flags.get(spv::DecorationNonReadable)) {
								opt_access = MTLBindingAccessReadWrite;
							} else {
								opt_access = MTLBindingAccessWriteOnly;
							}
						} else {
							opt_access = MTLBindingAccessReadOnly;
						}
					} break;

					case BT::Image:
					case BT::SampledImage: {
						primary.dataType = MTLDataTypeTexture;
					} break;

					case BT::Sampler: {
						primary.dataType = MTLDataTypeSampler;
					} break;

					default: {
						ERR_FAIL_V_MSG(ERR_CANT_CREATE, "Unexpected BaseType");
					} break;
				}

				// find array length
				if (basetype == BT::Image || basetype == BT::SampledImage) {
					auto a_type = compiler.get_type(res.type_id);

					if (a_type.array.size() > 0) {
						primary.arrayLength = a_type.array[0];
					}
					primary.isMultisampled = a_type.image.ms;
					bool is_depth = compiler.variable_is_depth_or_compare(res.id);

					auto image = a_type.image;
					primary.imageFormat = image.format;

					switch (image.dim) {
						case spv::Dim1D: {
							if (image.arrayed) {
								primary.textureType = MTLTextureType1DArray;
							} else {
								primary.textureType = MTLTextureType1D;
							}
						} break;
						case spv::DimSubpassData: {
							if (image.dim == spv::Dim2D) {
								// subpass input
								primary_binding = get_decoration(res.id, spv::DecorationInputAttachmentIndex);
							}
						} // fallthrough to spv::Dim2D
						case spv::Dim2D: {
							if (image.arrayed && image.ms) {
								primary.textureType = MTLTextureType2DMultisampleArray;
							} else if (image.arrayed) {
								primary.textureType = MTLTextureType2DArray;
							} else if (image.ms) {
								primary.textureType = MTLTextureType2DMultisample;
							} else {
								primary.textureType = MTLTextureType2D;
							}
						} break;
						case spv::Dim3D: {
							primary.textureType = MTLTextureType3D;
						} break;
						case spv::DimCube: {
							if (image.arrayed) {
								primary.textureType = MTLTextureTypeCube;
							}
						} break;
						case spv::DimRect: {
						} break;
						case spv::DimBuffer: {
							// VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
							primary.textureType = MTLTextureTypeTextureBuffer;
						} break;
						case spv::DimMax: {
							// Add all enumerations to silence the compiler warning
							// and generate future warnings, should a new one be
							// added.
						} break;
					}

					// try to determine the access type from the image
					switch (image.access) {
						case spv::AccessQualifierWriteOnly:
							opt_access = MTLBindingAccessWriteOnly;
							break;
						case spv::AccessQualifierReadWrite:
							opt_access = MTLBindingAccessReadWrite;
							break;
						case spv::AccessQualifierReadOnly:
							opt_access = MTLBindingAccessReadOnly;
						default:
							break;
					}
				}

				if (opt_access.has_value()) {
					primary.access = opt_access.value();
				} else {
					// not an image or the image.access qualifier is not set
					if (!compiler.has_decoration(res.id, spv::DecorationNonWritable)) {
						if (!compiler.has_decoration(res.id, spv::DecorationNonReadable)) {
							primary.access = MTLBindingAccessReadWrite;
						} else {
							primary.access = MTLBindingAccessWriteOnly;
						}
					} else {
						primary.access = MTLBindingAccessReadOnly;
					}
				}

				switch (primary.access) {
					case MTLBindingAccessReadOnly:
						primary.usage = MTLResourceUsageRead;
						break;
					case MTLBindingAccessWriteOnly:
						primary.usage = MTLResourceUsageWrite;
						break;
					case MTLBindingAccessReadWrite:
						primary.usage = MTLResourceUsageRead | MTLResourceUsageWrite;
						break;
				}

				if (primary_binding.has_value()) {
					primary.index = primary_binding.value();
				} else {
					primary.index = compiler.get_automatic_msl_resource_binding(res.id);
				}

				found->bindings[stage] = primary;

				// A sampled image contains two bindings, the primary
				// is to the image, and the secondary is to the associated
				// sampler.
				if (basetype == BT::SampledImage) {
					uint32_t binding = compiler.get_automatic_msl_resource_binding_secondary(res.id);
					if (binding != -1) {
						found->bindings_secondary[stage] = BindingInfo{
							.dataType = MTLDataTypeSampler,
							.index = binding,
							.access = MTLBindingAccessReadOnly,
						};
					}
				}

				if (basetype == BT::Image) {
					uint32_t binding = compiler.get_automatic_msl_resource_binding_secondary(res.id);
					if (binding != -1) {
						found->bindings_secondary[stage] = BindingInfo{
							.dataType = MTLDataTypePointer,
							.index = binding,
							.access = MTLBindingAccessReadWrite,
						};
					}
				}
			}
			return Error::OK;
		};

		if (!resources.uniform_buffers.empty()) {
			Error err = descriptor_bindings("uniform buffers", resources.uniform_buffers);
			ERR_FAIL_COND_V(err != OK, Result());
		}
		if (!resources.storage_buffers.empty()) {
			Error err = descriptor_bindings("storage buffers", resources.storage_buffers);
			ERR_FAIL_COND_V(err != OK, Result());
		}
		if (!resources.storage_images.empty()) {
			Error err = descriptor_bindings("storage images", resources.storage_images);
			ERR_FAIL_COND_V(err != OK, Result());
		}
		if (!resources.sampled_images.empty()) {
			Error err = descriptor_bindings("sampled images", resources.sampled_images);
			ERR_FAIL_COND_V(err != OK, Result());
		}
		if (!resources.separate_images.empty()) {
			Error err = descriptor_bindings("separate images", resources.separate_images);
			ERR_FAIL_COND_V(err != OK, Result());
		}
		if (!resources.separate_samplers.empty()) {
			Error err = descriptor_bindings("separate samplers", resources.separate_samplers);
			ERR_FAIL_COND_V(err != OK, Result());
		}
		if (!resources.subpass_inputs.empty()) {
			Error err = descriptor_bindings("subpass inputs", resources.subpass_inputs);
			ERR_FAIL_COND_V(err != OK, Result());
		}

		if (!resources.push_constant_buffers.empty()) {
			for (auto &res : resources.push_constant_buffers) {
				auto binding = compiler.get_automatic_msl_resource_binding(res.id);
				if (binding != -1) {
					bin_data.push_constant.used_stages |= 1 << stage;
					bin_data.push_constant.msl_binding[stage] = binding;
				}
			}
		}

		ERR_FAIL_COND_V_MSG(!resources.atomic_counters.empty(), Result(), "Atomic counters not supported");
		ERR_FAIL_COND_V_MSG(!resources.acceleration_structures.empty(), Result(), "Acceleration structures not supported");
		ERR_FAIL_COND_V_MSG(!resources.shader_record_buffers.empty(), Result(), "Shader record buffers not supported");

		if (!resources.stage_inputs.empty()) {
			for (auto &res : resources.stage_inputs) {
				auto a_base_type = compiler.get_type(res.base_type_id);
				auto basetype = a_base_type.basetype;
				if (basetype == BT::Struct) {
					auto struct_size = compiler.get_declared_struct_size(a_base_type);
					print_line("struct_size: " + itos(struct_size));
				}
				auto name = compiler.get_name(res.id);
				auto binding = compiler.get_automatic_msl_resource_binding(res.id);
				if (binding != -1) {
					bin_data.vertex_input_mask |= 1 << binding;
				}
			}
		}

		ShaderStageData stage_data;
		stage_data.stage = v.shader_stage;
		stage_data.source_size = source.size();

		// compress source
		stage_data.source_data.resize(Compression::get_max_compressed_buffer_size(source.size(), Compression::MODE_ZSTD));
		int dst_size = Compression::compress(stage_data.source_data.data(), reinterpret_cast<uint8_t const *>(source.data()), source.size(), Compression::MODE_ZSTD);
		stage_data.source_data.resize(dst_size);

		stage_data.entry_point_name = data::string(entry_point.name);
		bin_data.stages.push_back(stage_data);
	}

	auto serialized = cista::serialize<MODE>(bin_data);

	Vector<uint8_t> ret;
	ret.resize(serialized.size());
	uint8_t *binptr = ret.ptrw();

	memcpy(binptr, serialized.data(), serialized.size());

	return ret;
}

RDD::ShaderID RenderingDeviceDriverMetal::shader_create_from_bytecode(const Vector<uint8_t> &p_shader_binary, ShaderDescription &r_shader_desc, String &r_name) {
	r_shader_desc = {}; // Driver-agnostic.

	const uint8_t *binptr = p_shader_binary.ptr();
	uint32_t binsize = p_shader_binary.size();

	ShaderBinaryData const *binary_data = nullptr;
	try {
		binary_data = cista::deserialize<ShaderBinaryData, MODE>(binptr, binptr + binsize);
	} catch (cista::cista_exception &e) {
		// incompatible version
		ERR_FAIL_V(ShaderID());
	}
	std::string name = binary_data->shader_name.str();

	MTLCompileOptions *options = [MTLCompileOptions new];
	HashMap<ShaderStage, id<MTLLibrary>> libraries;
#if DEV_ENABLED
	HashMap<ShaderStage, NSString *> library_sources;
#endif
	for (auto &shader_data : binary_data->stages) {
		size_t bufsize = shader_data.source_size;
		uint8_t *buf = static_cast<uint8_t *>(memalloc(bufsize));
		int decoded_size = Compression::decompress(buf, bufsize, shader_data.source_data.data(), shader_data.source_data.size(), Compression::MODE_ZSTD);
		ERR_FAIL_COND_V(decoded_size != bufsize, ShaderID());

		NSString *source = [[NSString alloc] initWithBytesNoCopy:buf
														  length:bufsize
														encoding:NSUTF8StringEncoding
													 deallocator:^(void *ptr, NSUInteger size) {
													   memfree(ptr);
													 }];
#if DEV_ENABLED
		library_sources[shader_data.stage] = source;
#endif

		if ([source containsString:@"void ComputeAutoExposure("]) {
			// TODO: remove this hack once we determine how to fix the
			//  code generation.
			source = [source stringByReplacingOccurrencesOfString:@", device atomic_uint* rw_spd_global_atomic_atomic"
													   withString:@", volatile device atomic_uint* rw_spd_global_atomic_atomic"];
		}

		NSError *error = nil;
		auto library = [device newLibraryWithSource:source options:options error:&error];
		if (error != nil) {
			print_verbose(error.localizedDescription.UTF8String);
			ERR_FAIL_V_MSG(ShaderID(), "failed to compile Metal source");
		}
		libraries[shader_data.stage] = library;
	}

	LocalVector<UniformSet> uniform_sets;
	uniform_sets.resize(binary_data->uniforms.size());

	r_shader_desc.uniform_sets.resize(binary_data->uniforms.size());

	// create sets
	for (auto &uniform_set : binary_data->uniforms) {
		UniformSet &set = uniform_sets[uniform_set.index];
		set.uniforms.resize(uniform_set.uniforms.size());

		Vector<ShaderUniform> &uset = r_shader_desc.uniform_sets.write[uniform_set.index];
		uset.resize(uniform_set.uniforms.size());

		for (int i = 0; i < uniform_set.uniforms.size(); i++) {
			auto &uniform = uniform_set.uniforms[i];

			ShaderUniform su;
			su.type = uniform.type;
			su.writable = uniform.writable;
			su.length = uniform.length;
			su.binding = uniform.binding;
			su.stages = uniform.stages;
			uset.write[i] = su;

			UniformInfo ui;
			ui.binding = uniform.binding;
			ui.active_stages = uniform.active_stages;
			for (auto &kv : uniform.bindings) {
				ui.bindings.insert(kv.first, kv.second);
			}
			for (auto &kv : uniform.bindings_secondary) {
				ui.bindings_secondary.insert(kv.first, kv.second);
			}
			set.uniforms[i] = ui;
		}
	}
	for (auto &uniform_set : binary_data->uniforms) {
		UniformSet &set = uniform_sets[uniform_set.index];

		// make encoders
		for (ShaderStageData const &stage_data : binary_data->stages) {
			ShaderStage stage = stage_data.stage;
			NSMutableArray<MTLArgumentDescriptor *> *descriptors = [NSMutableArray new];

			for (UniformInfo const &uniform : set.uniforms) {
				BindingInfo const *binding_info = uniform.bindings.getptr(stage);
				if (binding_info == nullptr)
					continue;

				[descriptors addObject:binding_info->new_argument_descriptor()];
				BindingInfo const *secondary_binding_info = uniform.bindings_secondary.getptr(stage);
				if (secondary_binding_info != nullptr) {
					[descriptors addObject:secondary_binding_info->new_argument_descriptor()];
				}
			}

			if (descriptors.count == 0) {
				// no bindings
				continue;
			}
			// sort by index
			[descriptors sortUsingComparator:^NSComparisonResult(MTLArgumentDescriptor *a, MTLArgumentDescriptor *b) {
			  if (a.index < b.index) {
				  return NSOrderedAscending;
			  } else if (a.index > b.index) {
				  return NSOrderedDescending;
			  } else {
				  return NSOrderedSame;
			  }
			}];

			id<MTLArgumentEncoder> enc = [device newArgumentEncoderWithArguments:descriptors];
			set.encoders[stage] = enc;
			set.offsets[stage] = set.buffer_size;
			set.buffer_size += enc.encodedLength;
		}
	}

	r_shader_desc.specialization_constants.resize(binary_data->constants.size());
	for (int i = 0; i < binary_data->constants.size(); i++) {
		auto &c = binary_data->constants[i];

		ShaderSpecializationConstant sc;
		sc.type = c.type;
		sc.constant_id = c.constant_id;
		sc.int_value = c.int_value;
		sc.stages = c.stages;
		r_shader_desc.specialization_constants.write[i] = sc;
	}

	MDShader *shader = nullptr;
	if (binary_data->is_compute) {
		MDComputeShader *cs = new MDComputeShader(String(name.c_str()), uniform_sets, libraries[ShaderStage::SHADER_STAGE_COMPUTE]);

		std::optional<uint32_t> binding = binary_data->push_constant.msl_binding.get(SHADER_STAGE_COMPUTE);
		if (binding.has_value()) {
			cs->push_constants.size = binary_data->push_constant.size;
			cs->push_constants.binding = binding.value();
		}

		cs->local = MTLSizeMake(binary_data->compute_local_size.x, binary_data->compute_local_size.y, binary_data->compute_local_size.z);
#if DEV_ENABLED
		cs->kernel_source = library_sources[ShaderStage::SHADER_STAGE_COMPUTE];
#endif
		shader = cs;
	} else {
		MDRenderShader *rs = new MDRenderShader(String(name.c_str()), uniform_sets, libraries[ShaderStage::SHADER_STAGE_VERTEX], libraries[ShaderStage::SHADER_STAGE_FRAGMENT]);

		std::optional<uint32_t> vert_binding = binary_data->push_constant.msl_binding.get(SHADER_STAGE_VERTEX);
		if (vert_binding.has_value()) {
			rs->push_constants.vert.size = binary_data->push_constant.size;
			rs->push_constants.vert.binding = vert_binding.value();
		}
		std::optional<uint32_t> frag_binding = binary_data->push_constant.msl_binding.get(SHADER_STAGE_FRAGMENT);
		if (frag_binding.has_value()) {
			rs->push_constants.frag.size = binary_data->push_constant.size;
			rs->push_constants.frag.binding = frag_binding.value();
		}

#if DEV_ENABLED
		rs->vert_source = library_sources[ShaderStage::SHADER_STAGE_VERTEX];
		rs->frag_source = library_sources[ShaderStage::SHADER_STAGE_FRAGMENT];
#endif
		shader = rs;
	}

	r_shader_desc.vertex_input_mask = binary_data->vertex_input_mask;
	r_shader_desc.fragment_output_mask = binary_data->fragment_output_mask;
	r_shader_desc.is_compute = binary_data->is_compute;
	r_shader_desc.compute_local_size[0] = binary_data->compute_local_size.x;
	r_shader_desc.compute_local_size[1] = binary_data->compute_local_size.y;
	r_shader_desc.compute_local_size[2] = binary_data->compute_local_size.z;
	r_shader_desc.push_constant_size = binary_data->push_constant.size;

	return ShaderID(shader);
}

void RenderingDeviceDriverMetal::shader_free(ShaderID p_shader) {
	MDShader *obj = (MDShader *)p_shader.id;
	delete obj;
}

/*********************/
/**** UNIFORM SET ****/
/*********************/

RDD::UniformSetID RenderingDeviceDriverMetal::uniform_set_create(VectorView<BoundUniform> p_uniforms, ShaderID p_shader, uint32_t p_set_index) {
	MDUniformSet *set = new MDUniformSet();
	Vector<BoundUniform> bound_uniforms;
	bound_uniforms.resize(p_uniforms.size());
	for (int i = 0; i < p_uniforms.size(); i += 1) {
		bound_uniforms.write[i] = p_uniforms[i];
	}
	set->p_uniforms = bound_uniforms;
	set->index = p_set_index;

	return UniformSetID(set);
}

void RenderingDeviceDriverMetal::uniform_set_free(UniformSetID p_uniform_set) {
	MDUniformSet *obj = (MDUniformSet *)p_uniform_set.id;
	delete obj;
}

void RenderingDeviceDriverMetal::command_uniform_set_prepare_for_use(CommandBufferID p_cmd_buffer, UniformSetID p_uniform_set, ShaderID p_shader, uint32_t p_set_index) {
}

/******************/
/**** TRANSFER ****/
/******************/

void RenderingDeviceDriverMetal::command_clear_buffer(CommandBufferID p_cmd_buffer, BufferID p_buffer, uint64_t p_offset, uint64_t p_size) {
	MDCommandBuffer *cmd = (MDCommandBuffer *)(p_cmd_buffer.id);
	id<MTLBuffer> buffer = rid::get(p_buffer);

	id<MTLBlitCommandEncoder> blit = cmd->blit_command_encoder();
	[blit fillBuffer:buffer
			   range:NSMakeRange(p_offset, p_size)
			   value:0];
}

void RenderingDeviceDriverMetal::command_copy_buffer(CommandBufferID p_cmd_buffer, BufferID p_src_buffer, BufferID p_dst_buffer, VectorView<BufferCopyRegion> p_regions) {
	MDCommandBuffer *cmd = (MDCommandBuffer *)(p_cmd_buffer.id);
	id<MTLBuffer> src = rid::get(p_src_buffer);
	id<MTLBuffer> dst = rid::get(p_dst_buffer);

	id<MTLBlitCommandEncoder> blit = cmd->blit_command_encoder();

	for (uint32_t i = 0; i < p_regions.size(); i++) {
		BufferCopyRegion region = p_regions[i];
		[blit copyFromBuffer:src
					 sourceOffset:region.src_offset
						 toBuffer:dst
				destinationOffset:region.dst_offset
							 size:region.size];
	}
}

void RenderingDeviceDriverMetal::command_copy_texture(CommandBufferID p_cmd_buffer, TextureID p_src_texture, TextureLayout p_src_texture_layout, TextureID p_dst_texture, TextureLayout p_dst_texture_layout, VectorView<TextureCopyRegion> p_regions) {
	ERR_FAIL_MSG("not implemented");
}

void RenderingDeviceDriverMetal::command_resolve_texture(CommandBufferID p_cmd_buffer, TextureID p_src_texture, TextureLayout p_src_texture_layout, uint32_t p_src_layer, uint32_t p_src_mipmap, TextureID p_dst_texture, TextureLayout p_dst_texture_layout, uint32_t p_dst_layer, uint32_t p_dst_mipmap) {
	MDCommandBuffer *cb = (MDCommandBuffer *)(p_cmd_buffer.id);
	id<MTLTexture> src_tex = rid::get(p_src_texture);
	id<MTLTexture> dst_tex = rid::get(p_dst_texture);

	MTLRenderPassDescriptor *mtlRPD = [MTLRenderPassDescriptor renderPassDescriptor];
	MTLRenderPassColorAttachmentDescriptor *mtlColorAttDesc = mtlRPD.colorAttachments[0];
	mtlColorAttDesc.loadAction = MTLLoadActionLoad;
	mtlColorAttDesc.storeAction = MTLStoreActionMultisampleResolve;

	mtlColorAttDesc.texture = src_tex;
	mtlColorAttDesc.resolveTexture = dst_tex;
	mtlColorAttDesc.level = p_src_mipmap;
	mtlColorAttDesc.slice = p_src_layer;
	mtlColorAttDesc.resolveLevel = p_dst_mipmap;
	mtlColorAttDesc.resolveSlice = p_dst_layer;
	cb->encodeRenderCommandEncoderWithDescriptor(mtlRPD, @"Resolve Image");
}

void RenderingDeviceDriverMetal::command_clear_color_texture(CommandBufferID p_cmd_buffer, TextureID p_texture, TextureLayout p_texture_layout, const Color &p_color, const TextureSubresourceRange &p_subresources) {
	MDCommandBuffer *cb = (MDCommandBuffer *)(p_cmd_buffer.id);
	id<MTLTexture> src_tex = rid::get(p_texture);

	if (src_tex.parentTexture) {
		// clear via the parent texture rather than the view
		src_tex = src_tex.parentTexture;
	}

	PixelFormats &pf = context->get_pixel_formats();

	if (pf.isDepthFormat(src_tex.pixelFormat) || pf.isStencilFormat(src_tex.pixelFormat)) {
		ERR_FAIL_MSG("invalid: depth or stencil texture format");
	}

	MTLRenderPassDescriptor *desc = MTLRenderPassDescriptor.renderPassDescriptor;

	if (p_subresources.aspect.has_flag(TEXTURE_ASPECT_COLOR_BIT)) {
		MTLRenderPassColorAttachmentDescriptor *caDesc = desc.colorAttachments[0];
		caDesc.texture = src_tex;
		caDesc.loadAction = MTLLoadActionClear;
		caDesc.storeAction = MTLStoreActionStore;
		caDesc.clearColor = MTLClearColorMake(p_color.r, p_color.g, p_color.b, p_color.a);

		// Extract the mipmap levels that are to be updated
		uint32_t mipLvlStart = p_subresources.base_mipmap;
		uint32_t mipLvlCnt = p_subresources.mipmap_count;
		uint32_t mipLvlEnd = mipLvlStart + mipLvlCnt;

		uint32_t levelCount = src_tex.mipmapLevelCount;

		// Extract the cube or array layers (slices) that are to be updated
		bool is3D = src_tex.textureType == MTLTextureType3D;
		uint32_t layerStart = is3D ? 0 : p_subresources.base_layer;
		uint32_t layerCnt = p_subresources.layer_count;
		uint32_t layerEnd = layerStart + layerCnt;

		MetalFeatures const &features = context->get_device_properties().features;

		// Iterate across mipmap levels and layers, and perform and empty render to clear each
		for (uint32_t mipLvl = mipLvlStart; mipLvl < mipLvlEnd; mipLvl++) {
			ERR_FAIL_INDEX_MSG(mipLvl, levelCount, "mip level out of range");

			caDesc.level = mipLvl;

			// If a 3D image, we need to get the depth for each level.
			if (is3D) {
				layerCnt = mipmapLevelSizeFromTexture(src_tex, mipLvl).depth;
				layerEnd = layerStart + layerCnt;
			}

			if (features.layeredRendering && src_tex.sampleCount == 1 || features.multisampleLayeredRendering) {
				// we can clear all layers at once
				if (is3D) {
					caDesc.depthPlane = layerStart;
				} else {
					caDesc.slice = layerStart;
				}
				desc.renderTargetArrayLength = layerCnt;
				cb->encodeRenderCommandEncoderWithDescriptor(desc, @"Clear Image");
			} else {
				for (uint32_t layer = layerStart; layer < layerEnd; layer++) {
					if (is3D) {
						caDesc.depthPlane = layer;
					} else {
						caDesc.slice = layer;
					}
					cb->encodeRenderCommandEncoderWithDescriptor(desc, @"Clear Image");
				}
			}
		}
	}
}

bool isArrayTexture(MTLTextureType mtlTexType) {
	return (mtlTexType == MTLTextureType3D ||
			mtlTexType == MTLTextureType2DArray ||
			mtlTexType == MTLTextureType2DMultisampleArray ||
			mtlTexType == MTLTextureType1DArray);
}

void RenderingDeviceDriverMetal::command_copy_buffer_to_texture(CommandBufferID p_cmd_buffer, BufferID p_src_buffer, TextureID p_dst_texture, TextureLayout p_dst_texture_layout, VectorView<BufferTextureCopyRegion> p_regions) {
	MDCommandBuffer *cmd = (MDCommandBuffer *)(p_cmd_buffer.id);
	id<MTLBuffer> buffer = rid::get(p_src_buffer);
	id<MTLTexture> texture = rid::get(p_dst_texture);

	id<MTLBlitCommandEncoder> enc = cmd->blit_command_encoder();

	PixelFormats &pf = context->get_pixel_formats();
	MTLPixelFormat mtlPixFmt = texture.pixelFormat;

	MTLBlitOption options = MTLBlitOptionNone;
	if (pf.isPVRTCFormat(mtlPixFmt)) {
		options |= MTLBlitOptionRowLinearPVRTC;
	}

	for (uint32_t i = 0; i < p_regions.size(); i++) {
		BufferTextureCopyRegion region = p_regions[i];

		uint32_t buffImgWd = region.texture_region_size.x;
		uint32_t buffImgHt = region.texture_region_size.y;

		NSUInteger bytesPerRow = pf.getBytesPerRow(mtlPixFmt, buffImgWd);
		NSUInteger bytesPerImg = 0;
		if (!isArrayTexture(texture.textureType)) {
			bytesPerImg = pf.getBytesPerLayer(mtlPixFmt, bytesPerRow, buffImgHt);
		}

		[enc copyFromBuffer:buffer
					   sourceOffset:region.buffer_offset
				  sourceBytesPerRow:bytesPerRow
				sourceBytesPerImage:bytesPerImg
						 sourceSize:MTLSizeMake(region.texture_region_size.x, region.texture_region_size.y, region.texture_region_size.z)
						  toTexture:texture
				   destinationSlice:region.texture_subresources.base_layer
				   destinationLevel:region.texture_subresources.mipmap
				  destinationOrigin:MTLOriginMake(region.texture_offset.x, region.texture_offset.y, region.texture_offset.z)
							options:options];
	}
}

void RenderingDeviceDriverMetal::command_copy_texture_to_buffer(CommandBufferID p_cmd_buffer, TextureID p_src_texture, TextureLayout p_src_texture_layout, BufferID p_dst_buffer, VectorView<BufferTextureCopyRegion> p_regions) {
	MDCommandBuffer *cmd = (MDCommandBuffer *)(p_cmd_buffer.id);
	id<MTLTexture> texture = rid::get(p_src_texture);
	id<MTLBuffer> buffer = rid::get(p_dst_buffer);

	id<MTLBlitCommandEncoder> enc = cmd->blit_command_encoder();

	PixelFormats &pf = context->get_pixel_formats();
	MTLPixelFormat mtlPixFmt = texture.pixelFormat;

	MTLBlitOption options = MTLBlitOptionNone;
	if (pf.isPVRTCFormat(mtlPixFmt)) {
		options |= MTLBlitOptionRowLinearPVRTC;
	}

	for (uint32_t i = 0; i < p_regions.size(); i++) {
		BufferTextureCopyRegion region = p_regions[i];

		uint32_t buffImgWd = region.texture_region_size.x;
		uint32_t buffImgHt = region.texture_region_size.y;

		NSUInteger bytesPerRow = pf.getBytesPerRow(mtlPixFmt, buffImgWd);
		NSUInteger bytesPerImg = 0;
		if (!isArrayTexture(texture.textureType)) {
			bytesPerImg = pf.getBytesPerLayer(mtlPixFmt, bytesPerRow, buffImgHt);
		}

		[enc copyFromTexture:texture
							 sourceSlice:region.texture_subresources.base_layer
							 sourceLevel:region.texture_subresources.mipmap
							sourceOrigin:MTLOriginMake(region.texture_offset.x, region.texture_offset.y, region.texture_offset.z)
							  sourceSize:MTLSizeMake(region.texture_region_size.x, region.texture_region_size.y, region.texture_region_size.z)
								toBuffer:buffer
					   destinationOffset:region.buffer_offset
				  destinationBytesPerRow:bytesPerRow
				destinationBytesPerImage:bytesPerImg
								 options:options];
	}
}

/******************/
/**** PIPELINE ****/
/******************/

void RenderingDeviceDriverMetal::pipeline_free(PipelineID p_pipeline_id) {
	MDPipeline *obj = (MDPipeline *)(p_pipeline_id.id);
	delete obj;
}

// ----- BINDING -----

void RenderingDeviceDriverMetal::command_bind_push_constants(CommandBufferID p_cmd_buffer, ShaderID p_shader, uint32_t p_dst_first_index, VectorView<uint32_t> p_data) {
	MDCommandBuffer *cb = (MDCommandBuffer *)(p_cmd_buffer.id);
	MDShader *shader = (MDShader *)(p_shader.id);
	shader->encode_push_constant_data(p_data, cb);
}

// ----- CACHE -----

bool RenderingDeviceDriverMetal::pipeline_cache_create(const Vector<uint8_t> &p_data) {
	return false;
}

void RenderingDeviceDriverMetal::pipeline_cache_free() {
}

size_t RenderingDeviceDriverMetal::pipeline_cache_query_size() {
	ERR_FAIL_V_MSG(0, "not implemented");
}

Vector<uint8_t> RenderingDeviceDriverMetal::pipeline_cache_serialize() {
	ERR_FAIL_V_MSG(Vector<uint8_t>(), "not implemented");
}

/*******************/
/**** RENDERING ****/
/*******************/

// ----- SUBPASS -----

RDD::RenderPassID RenderingDeviceDriverMetal::render_pass_create(VectorView<Attachment> p_attachments, VectorView<Subpass> p_subpasses, VectorView<SubpassDependency> p_subpass_dependencies, uint32_t p_view_count) {
	PixelFormats &pf = context->get_pixel_formats();

	size_t subpass_count = p_subpasses.size();

	TightLocalVector<MDSubpass> subpasses;
	subpasses.resize(subpass_count);
	for (uint32_t i = 0; i < subpass_count; i++) {
		subpasses[i].subpass_index = i;
		subpasses[i].input_references = p_subpasses[i].input_references;
		subpasses[i].color_references = p_subpasses[i].color_references;
		subpasses[i].depth_stencil_reference = p_subpasses[i].depth_stencil_reference;
		subpasses[i].resolve_references = p_subpasses[i].resolve_references;
	}

	static const MTLLoadAction loadActions[] = {
		[ATTACHMENT_LOAD_OP_LOAD] = MTLLoadActionLoad,
		[ATTACHMENT_LOAD_OP_CLEAR] = MTLLoadActionClear,
		[ATTACHMENT_LOAD_OP_DONT_CARE] = MTLLoadActionDontCare,
	};

	static const MTLStoreAction storeActions[] = {
		[ATTACHMENT_STORE_OP_STORE] = MTLStoreActionStore,
		[ATTACHMENT_STORE_OP_DONT_CARE] = MTLStoreActionDontCare,
	};

	TightLocalVector<MDAttachment> attachments;
	attachments.resize(p_attachments.size());

	for (uint32_t i = 0; i < p_attachments.size(); i++) {
		Attachment const &a = p_attachments[i];
		MDAttachment *mda = &attachments[i];
		MTLPixelFormat format = pf.getMTLPixelFormat(a.format);
		mda->format = format;
		if (a.samples > TEXTURE_SAMPLES_1) {
			mda->samples = context->get_device_properties().find_nearest_supported_sample_count(a.samples);
		}
		mda->loadAction = loadActions[a.load_op];
		mda->storeAction = storeActions[a.store_op];
		bool is_depth = pf.isDepthFormat(format);
		if (is_depth) {
			mda->type |= MDAttachmentType::Depth;
		}
		bool is_stencil = pf.isStencilFormat(format);
		if (is_stencil) {
			mda->type |= MDAttachmentType::Stencil;
		}
		if (!is_depth && !is_stencil) {
			mda->type |= MDAttachmentType::Color;
		}
	}
	MDRenderPass *obj = new MDRenderPass(std::move(attachments), std::move(subpasses));
	return RenderPassID(obj);
}

void RenderingDeviceDriverMetal::render_pass_free(RenderPassID p_render_pass) {
	MDRenderPass *obj = (MDRenderPass *)(p_render_pass.id);
	delete obj;
}

// ----- COMMANDS -----

void RenderingDeviceDriverMetal::command_begin_render_pass(CommandBufferID p_cmd_buffer, RenderPassID p_render_pass, FramebufferID p_framebuffer, CommandBufferType p_cmd_buffer_type, const Rect2i &p_rect, VectorView<RenderPassClearValue> p_clear_values) {
	MDCommandBuffer *cb = (MDCommandBuffer *)(p_cmd_buffer.id);
	cb->render_begin_pass(p_render_pass, p_framebuffer, p_cmd_buffer_type, p_rect, p_clear_values);
}

void RenderingDeviceDriverMetal::command_end_render_pass(CommandBufferID p_cmd_buffer) {
	MDCommandBuffer *cb = (MDCommandBuffer *)(p_cmd_buffer.id);
	cb->render_end_pass();
}

void RenderingDeviceDriverMetal::command_next_render_subpass(CommandBufferID p_cmd_buffer, CommandBufferType p_cmd_buffer_type) {
	MDCommandBuffer *cb = (MDCommandBuffer *)(p_cmd_buffer.id);
	cb->render_next_subpass();
}

void RenderingDeviceDriverMetal::command_render_set_viewport(CommandBufferID p_cmd_buffer, VectorView<Rect2i> p_viewports) {
	MDCommandBuffer *cb = (MDCommandBuffer *)(p_cmd_buffer.id);
	cb->render_set_viewport(p_viewports);
}

void RenderingDeviceDriverMetal::command_render_set_scissor(CommandBufferID p_cmd_buffer, VectorView<Rect2i> p_scissors) {
	MDCommandBuffer *cb = (MDCommandBuffer *)(p_cmd_buffer.id);
	cb->render_set_scissor(p_scissors);
}

void RenderingDeviceDriverMetal::command_render_clear_attachments(CommandBufferID p_cmd_buffer, VectorView<AttachmentClear> p_attachment_clears, VectorView<Rect2i> p_rects) {
	MDCommandBuffer *cb = (MDCommandBuffer *)(p_cmd_buffer.id);
	cb->render_clear_attachments(p_attachment_clears, p_rects);
}

void RenderingDeviceDriverMetal::command_bind_render_pipeline(CommandBufferID p_cmd_buffer, PipelineID p_pipeline) {
	MDCommandBuffer *cb = (MDCommandBuffer *)(p_cmd_buffer.id);
	cb->bind_pipeline(p_pipeline);
}

void RenderingDeviceDriverMetal::command_bind_render_uniform_set(CommandBufferID p_cmd_buffer, UniformSetID p_uniform_set, ShaderID p_shader, uint32_t p_set_index) {
	MDCommandBuffer *cb = (MDCommandBuffer *)(p_cmd_buffer.id);
	cb->render_bind_uniform_set(p_uniform_set, p_shader, p_set_index);
}

void RenderingDeviceDriverMetal::command_render_draw(CommandBufferID p_cmd_buffer, uint32_t p_vertex_count, uint32_t p_instance_count, uint32_t p_base_vertex, uint32_t p_first_instance) {
	MDCommandBuffer *cb = (MDCommandBuffer *)(p_cmd_buffer.id);
	cb->render_draw(p_vertex_count, p_instance_count, p_base_vertex, p_first_instance);
}

void RenderingDeviceDriverMetal::command_render_draw_indexed(CommandBufferID p_cmd_buffer, uint32_t p_index_count, uint32_t p_instance_count, uint32_t p_first_index, int32_t p_vertex_offset, uint32_t p_first_instance) {
	MDCommandBuffer *cb = (MDCommandBuffer *)(p_cmd_buffer.id);
	cb->render_draw_indexed(p_index_count, p_instance_count, p_first_index, p_vertex_offset, p_first_instance);
}

void RenderingDeviceDriverMetal::command_render_draw_indexed_indirect(CommandBufferID p_cmd_buffer, BufferID p_indirect_buffer, uint64_t p_offset, uint32_t p_draw_count, uint32_t p_stride) {
	ERR_FAIL_MSG("not implemented");
}

void RenderingDeviceDriverMetal::command_render_draw_indexed_indirect_count(CommandBufferID p_cmd_buffer, BufferID p_indirect_buffer, uint64_t p_offset, BufferID p_count_buffer, uint64_t p_count_buffer_offset, uint32_t p_max_draw_count, uint32_t p_stride) {
	ERR_FAIL_MSG("not implemented");
}

void RenderingDeviceDriverMetal::command_render_draw_indirect(CommandBufferID p_cmd_buffer, BufferID p_indirect_buffer, uint64_t p_offset, uint32_t p_draw_count, uint32_t p_stride) {
	ERR_FAIL_MSG("not implemented");
}

void RenderingDeviceDriverMetal::command_render_draw_indirect_count(CommandBufferID p_cmd_buffer, BufferID p_indirect_buffer, uint64_t p_offset, BufferID p_count_buffer, uint64_t p_count_buffer_offset, uint32_t p_max_draw_count, uint32_t p_stride) {
	ERR_FAIL_MSG("not implemented");
}

void RenderingDeviceDriverMetal::command_render_bind_vertex_buffers(CommandBufferID p_cmd_buffer, uint32_t p_binding_count, const BufferID *p_buffers, const uint64_t *p_offsets) {
	MDCommandBuffer *cb = (MDCommandBuffer *)(p_cmd_buffer.id);
	cb->render_bind_vertex_buffers(p_binding_count, p_buffers, p_offsets);
}

void RenderingDeviceDriverMetal::command_render_bind_index_buffer(CommandBufferID p_cmd_buffer, BufferID p_buffer, IndexBufferFormat p_format, uint64_t p_offset) {
	MDCommandBuffer *cb = (MDCommandBuffer *)(p_cmd_buffer.id);
	cb->render_bind_index_buffer(p_buffer, p_format, p_offset);
}

void RenderingDeviceDriverMetal::command_render_set_blend_constants(CommandBufferID p_cmd_buffer, const Color &p_constants) {
	ERR_FAIL_MSG("not implemented");
}

void RenderingDeviceDriverMetal::command_render_set_line_width(CommandBufferID p_cmd_buffer, float p_width) {
	ERR_FAIL_MSG("not implemented");
}

// ----- PIPELINE -----

RDM::Result<id<MTLFunction>> RenderingDeviceDriverMetal::_create_function(id<MTLLibrary> p_library, NSString *p_name, VectorView<PipelineSpecializationConstant> &p_specialization_constants) {
	id<MTLFunction> function = [p_library newFunctionWithName:@"main0"];

	if (function.functionConstantsDictionary.count == 0) {
		function = [p_library newFunctionWithName:@"main0"];
		ERR_FAIL_NULL_V_MSG(function, ERR_CANT_CREATE, "No function named main0");
		return function;
	}

	NSArray<MTLFunctionConstant *> *constants = function.functionConstantsDictionary.allValues;
	bool is_sorted = true;
	for (int i = 1; i < constants.count; i++) {
		if (constants[i - 1].index < constants[i].index) {
			is_sorted = false;
			break;
		}
	}

	if (!is_sorted) {
		constants = [constants sortedArrayUsingComparator:^NSComparisonResult(MTLFunctionConstant *a, MTLFunctionConstant *b) {
		  if (a.index < b.index) {
			  return NSOrderedAscending;
		  } else if (a.index > b.index) {
			  return NSOrderedDescending;
		  } else {
			  return NSOrderedSame;
		  }
		}];
	}

	MTLFunctionConstantValues *constantValues = [MTLFunctionConstantValues new];
	int i = 0;
	int j = 0;
	while (i < constants.count && j < p_specialization_constants.size()) {
		MTLFunctionConstant *curr = constants[i];
		PipelineSpecializationConstant const &sc = p_specialization_constants[j];
		if (curr.index == sc.constant_id) {
			switch (curr.type) {
				case MTLDataTypeBool:
				case MTLDataTypeFloat:
				case MTLDataTypeInt:
				case MTLDataTypeUInt: {
					[constantValues setConstantValue:&sc.int_value
												type:curr.type
											 atIndex:sc.constant_id];
				} break;
				default:
					ERR_FAIL_V_MSG(function, "Invalid specialization constant type");
			}
			i++;
			j++;
		} else if (curr.index < sc.constant_id) {
			i++;
		} else {
			j++;
		}
	}

	if (i != constants.count) {
		MTLFunctionConstant *curr = constants[i];
		if (curr.index == R32UI_ALIGNMENT_CONSTANT_ID) {
			uint32_t alignment = 16; // TODO(sgc): is this correct?
			[constantValues setConstantValue:&alignment
										type:curr.type
									 atIndex:curr.index];
			i++;
		}
	}

	if (i != constants.count) {
		ERR_FAIL_V_MSG(function, "Not all specialization constants set for function");
	}

	NSError *err = nil;
	function = [p_library newFunctionWithName:@"main0"
							   constantValues:constantValues
										error:&err];
	ERR_FAIL_NULL_V_EDMSG(function, ERR_CANT_CREATE, String("specialized function failed: ") + err.localizedDescription.UTF8String);

	return function;
}

// RDD::PolygonCullMode == VkCullModeFlagBits.
static_assert(ENUM_MEMBERS_EQUAL(RDD::POLYGON_CULL_DISABLED, MTLCullModeNone));
static_assert(ENUM_MEMBERS_EQUAL(RDD::POLYGON_CULL_FRONT, MTLCullModeFront));
static_assert(ENUM_MEMBERS_EQUAL(RDD::POLYGON_CULL_BACK, MTLCullModeBack));

// RDD::StencilOperation == VkStencilOp.
static_assert(ENUM_MEMBERS_EQUAL(RDD::STENCIL_OP_KEEP, MTLStencilOperationKeep));
static_assert(ENUM_MEMBERS_EQUAL(RDD::STENCIL_OP_ZERO, MTLStencilOperationZero));
static_assert(ENUM_MEMBERS_EQUAL(RDD::STENCIL_OP_REPLACE, MTLStencilOperationReplace));
static_assert(ENUM_MEMBERS_EQUAL(RDD::STENCIL_OP_INCREMENT_AND_CLAMP, MTLStencilOperationIncrementClamp));
static_assert(ENUM_MEMBERS_EQUAL(RDD::STENCIL_OP_DECREMENT_AND_CLAMP, MTLStencilOperationDecrementClamp));
static_assert(ENUM_MEMBERS_EQUAL(RDD::STENCIL_OP_INVERT, MTLStencilOperationInvert));
static_assert(ENUM_MEMBERS_EQUAL(RDD::STENCIL_OP_INCREMENT_AND_WRAP, MTLStencilOperationIncrementWrap));
static_assert(ENUM_MEMBERS_EQUAL(RDD::STENCIL_OP_DECREMENT_AND_WRAP, MTLStencilOperationDecrementWrap));

// RDD::BlendOperation == VkBlendOp.
static_assert(ENUM_MEMBERS_EQUAL(RDD::BLEND_OP_ADD, MTLBlendOperationAdd));
static_assert(ENUM_MEMBERS_EQUAL(RDD::BLEND_OP_SUBTRACT, MTLBlendOperationSubtract));
static_assert(ENUM_MEMBERS_EQUAL(RDD::BLEND_OP_REVERSE_SUBTRACT, MTLBlendOperationReverseSubtract));
static_assert(ENUM_MEMBERS_EQUAL(RDD::BLEND_OP_MINIMUM, MTLBlendOperationMin));
static_assert(ENUM_MEMBERS_EQUAL(RDD::BLEND_OP_MAXIMUM, MTLBlendOperationMax));

RDD::PipelineID RenderingDeviceDriverMetal::render_pipeline_create(
		ShaderID p_shader,
		VertexFormatID p_vertex_format,
		RenderPrimitive p_render_primitive,
		PipelineRasterizationState p_rasterization_state,
		PipelineMultisampleState p_multisample_state,
		PipelineDepthStencilState p_depth_stencil_state,
		PipelineColorBlendState p_blend_state,
		VectorView<int32_t> p_color_attachments,
		BitField<PipelineDynamicStateFlags> p_dynamic_state,
		RenderPassID p_render_pass,
		uint32_t p_render_subpass,
		VectorView<PipelineSpecializationConstant> p_specialization_constants) {
	MDRenderShader *shader = (MDRenderShader *)(p_shader.id);
	MTLVertexDescriptor *vert_desc = rid::get(p_vertex_format);
	MDRenderPass *pass = (MDRenderPass *)(p_render_pass.id);

	PixelFormats &pf = context->get_pixel_formats();
	MTLRenderPipelineDescriptor *desc = [MTLRenderPipelineDescriptor new];

	{
		MDSubpass const &subpass = pass->subpasses[p_render_subpass];
		for (int i = 0; i < subpass.color_references.size(); i++) {
			int32_t attachment = subpass.color_references[i].attachment;
			if (attachment != ATTACHMENT_UNUSED) {
				MDAttachment const &a = pass->attachments[attachment];
				desc.colorAttachments[i].pixelFormat = a.format;
			}
		}

		if (subpass.depth_stencil_reference.attachment != ATTACHMENT_UNUSED) {
			int32_t attachment = subpass.depth_stencil_reference.attachment;
			MDAttachment const &a = pass->attachments[attachment];

			if (a.type & MDAttachmentType::Depth) {
				desc.depthAttachmentPixelFormat = a.format;
			}

			if (a.type & MDAttachmentType::Stencil) {
				desc.stencilAttachmentPixelFormat = a.format;
			}
		}
	}

	desc.vertexDescriptor = vert_desc;

	// Input assembly & tessellation.

	static const MTLPrimitiveTopologyClass topology_classes[RENDER_PRIMITIVE_MAX] = {
		MTLPrimitiveTopologyClassPoint,
		MTLPrimitiveTopologyClassLine,
		MTLPrimitiveTopologyClassUnspecified,
		MTLPrimitiveTopologyClassLine,
		MTLPrimitiveTopologyClassUnspecified,
		MTLPrimitiveTopologyClassTriangle,
		MTLPrimitiveTopologyClassUnspecified,
		MTLPrimitiveTopologyClassTriangle,
		MTLPrimitiveTopologyClassUnspecified,
		MTLPrimitiveTopologyClassUnspecified,
		MTLPrimitiveTopologyClassUnspecified,
	};

	MDRenderPipeline *pipeline = new MDRenderPipeline();

	// set topology
	switch (p_render_primitive) {
		case RENDER_PRIMITIVE_POINTS:
			desc.inputPrimitiveTopology = MTLPrimitiveTopologyClassPoint;
			break;
		case RENDER_PRIMITIVE_LINES:
		case RENDER_PRIMITIVE_LINES_WITH_ADJACENCY:
		case RENDER_PRIMITIVE_LINESTRIPS_WITH_ADJACENCY:
		case RENDER_PRIMITIVE_LINESTRIPS:
			desc.inputPrimitiveTopology = MTLPrimitiveTopologyClassLine;
			break;
		case RENDER_PRIMITIVE_TRIANGLES:
		case RENDER_PRIMITIVE_TRIANGLE_STRIPS:
		case RENDER_PRIMITIVE_TRIANGLES_WITH_ADJACENCY:
		case RENDER_PRIMITIVE_TRIANGLE_STRIPS_WITH_AJACENCY:
		case RENDER_PRIMITIVE_TRIANGLE_STRIPS_WITH_RESTART_INDEX:
			desc.inputPrimitiveTopology = MTLPrimitiveTopologyClassTriangle;
			break;
		case RENDER_PRIMITIVE_TESSELATION_PATCH:
			desc.maxTessellationFactor = p_rasterization_state.patch_control_points;
			desc.tessellationPartitionMode = MTLTessellationPartitionModeInteger;
			ERR_FAIL_V_MSG(PipelineID(), "tesselation not implemented");
			break;
		case RENDER_PRIMITIVE_MAX:
		default:
			desc.inputPrimitiveTopology = MTLPrimitiveTopologyClassUnspecified;
			break;
	}

	// set primitive
	switch (p_render_primitive) {
		case RENDER_PRIMITIVE_POINTS:
			pipeline->raster_state.render_primitive = MTLPrimitiveTypePoint;
			break;
		case RENDER_PRIMITIVE_LINES:
		case RENDER_PRIMITIVE_LINES_WITH_ADJACENCY:
			pipeline->raster_state.render_primitive = MTLPrimitiveTypeLine;
			break;
		case RENDER_PRIMITIVE_LINESTRIPS:
		case RENDER_PRIMITIVE_LINESTRIPS_WITH_ADJACENCY:
			pipeline->raster_state.render_primitive = MTLPrimitiveTypeLineStrip;
			break;
		case RENDER_PRIMITIVE_TRIANGLES:
		case RENDER_PRIMITIVE_TRIANGLES_WITH_ADJACENCY:
			pipeline->raster_state.render_primitive = MTLPrimitiveTypeTriangle;
			break;
		case RENDER_PRIMITIVE_TRIANGLE_STRIPS:
		case RENDER_PRIMITIVE_TRIANGLE_STRIPS_WITH_AJACENCY:
		case RENDER_PRIMITIVE_TRIANGLE_STRIPS_WITH_RESTART_INDEX:
			pipeline->raster_state.render_primitive = MTLPrimitiveTypeTriangleStrip;
			break;
		default:
			break;
	}

	// Rasterization.
	desc.rasterizationEnabled = !p_rasterization_state.discard_primitives;
	pipeline->raster_state.clip_mode = p_rasterization_state.enable_depth_clamp ? MTLDepthClipModeClamp : MTLDepthClipModeClip;
	pipeline->raster_state.fill_mode = p_rasterization_state.wireframe ? MTLTriangleFillModeLines : MTLTriangleFillModeFill;

	static const MTLCullMode cull_mode[3] = {
		MTLCullModeNone,
		MTLCullModeFront,
		MTLCullModeBack,
	};
	pipeline->raster_state.cull_mode = cull_mode[p_rasterization_state.cull_mode];
	pipeline->raster_state.winding = (p_rasterization_state.front_face == POLYGON_FRONT_FACE_CLOCKWISE) ? MTLWindingClockwise : MTLWindingCounterClockwise;
	pipeline->raster_state.depth_bias.enabled = p_rasterization_state.depth_bias_enabled;
	pipeline->raster_state.depth_bias.depth_bias = p_rasterization_state.depth_bias_constant_factor;
	pipeline->raster_state.depth_bias.slope_scale = p_rasterization_state.depth_bias_slope_factor;
	pipeline->raster_state.depth_bias.clamp = p_rasterization_state.depth_bias_clamp;
	// In Metal there is no line width
	if (!Math::is_equal_approx(p_rasterization_state.line_width, 1.0f)) {
		WARN_PRINT_ED("unsupported: line width");
	}

	// Multisample.
	if (p_multisample_state.enable_sample_shading) {
		WARN_PRINT_ED("unsupported: multi-sample shading");
	}

	if (p_multisample_state.sample_count > TEXTURE_SAMPLES_1) {
		pipeline->sample_count = context->get_device_properties().find_nearest_supported_sample_count(p_multisample_state.sample_count);
	}
	desc.rasterSampleCount = static_cast<NSUInteger>(pipeline->sample_count);
	desc.alphaToCoverageEnabled = p_multisample_state.enable_alpha_to_coverage;
	desc.alphaToOneEnabled = p_multisample_state.enable_alpha_to_one;

	// Depth stencil.
	if (p_depth_stencil_state.enable_depth_test && desc.depthAttachmentPixelFormat != MTLPixelFormatInvalid) {
		pipeline->raster_state.depth_test.enabled = true;
		MTLDepthStencilDescriptor *ds_desc = [MTLDepthStencilDescriptor new];
		ds_desc.depthWriteEnabled = p_depth_stencil_state.enable_depth_write;
		ds_desc.depthCompareFunction = compare_operators[p_depth_stencil_state.depth_compare_operator];
		if (p_depth_stencil_state.enable_depth_range) {
			WARN_PRINT_ED("unsupported: depth range");
		}

		if (p_depth_stencil_state.enable_stencil) {
			pipeline->raster_state.stencil.front_reference = p_depth_stencil_state.front_op.reference;
			pipeline->raster_state.stencil.back_reference = p_depth_stencil_state.back_op.reference;

			{
				// Front
				MTLStencilDescriptor *sd = [MTLStencilDescriptor new];
				sd.stencilFailureOperation = stencil_operations[p_depth_stencil_state.front_op.fail];
				sd.depthStencilPassOperation = stencil_operations[p_depth_stencil_state.front_op.pass];
				sd.depthFailureOperation = stencil_operations[p_depth_stencil_state.front_op.depth_fail];
				sd.stencilCompareFunction = compare_operators[p_depth_stencil_state.front_op.compare];
				sd.readMask = p_depth_stencil_state.front_op.compare_mask;
				sd.writeMask = p_depth_stencil_state.front_op.write_mask;
				ds_desc.frontFaceStencil = sd;
			}
			{
				// Back
				MTLStencilDescriptor *sd = [MTLStencilDescriptor new];
				sd.stencilFailureOperation = stencil_operations[p_depth_stencil_state.back_op.fail];
				sd.depthStencilPassOperation = stencil_operations[p_depth_stencil_state.back_op.pass];
				sd.depthFailureOperation = stencil_operations[p_depth_stencil_state.back_op.depth_fail];
				sd.stencilCompareFunction = compare_operators[p_depth_stencil_state.back_op.compare];
				sd.readMask = p_depth_stencil_state.back_op.compare_mask;
				sd.writeMask = p_depth_stencil_state.back_op.write_mask;
				ds_desc.backFaceStencil = sd;
			}
		}

		pipeline->depth_stencil = [device newDepthStencilStateWithDescriptor:ds_desc];
		ERR_FAIL_NULL_V_MSG(pipeline->depth_stencil, PipelineID(), "Failed to create depth stencil state");
	}

	// Blend state.
	{
		for (int i = 0; i < p_color_attachments.size(); i++) {
			if (p_color_attachments[i] == ATTACHMENT_UNUSED)
				continue;

			const PipelineColorBlendState::Attachment &bs = p_blend_state.attachments[i];

			MTLRenderPipelineColorAttachmentDescriptor *ca_desc = desc.colorAttachments[p_color_attachments[i]];
			ca_desc.blendingEnabled = bs.enable_blend;

			ca_desc.sourceRGBBlendFactor = blend_factors[bs.src_color_blend_factor];
			ca_desc.destinationRGBBlendFactor = blend_factors[bs.dst_color_blend_factor];
			ca_desc.rgbBlendOperation = blend_operations[bs.color_blend_op];

			ca_desc.sourceAlphaBlendFactor = blend_factors[bs.src_alpha_blend_factor];
			ca_desc.destinationAlphaBlendFactor = blend_factors[bs.dst_alpha_blend_factor];
			ca_desc.alphaBlendOperation = blend_operations[bs.alpha_blend_op];

			ca_desc.writeMask = MTLColorWriteMaskNone;
			if (bs.write_r) {
				ca_desc.writeMask |= MTLColorWriteMaskRed;
			}
			if (bs.write_g) {
				ca_desc.writeMask |= MTLColorWriteMaskGreen;
			}
			if (bs.write_b) {
				ca_desc.writeMask |= MTLColorWriteMaskBlue;
			}
			if (bs.write_a) {
				ca_desc.writeMask |= MTLColorWriteMaskAlpha;
			}
		}

		pipeline->raster_state.blend.r = p_blend_state.blend_constant.r;
		pipeline->raster_state.blend.g = p_blend_state.blend_constant.g;
		pipeline->raster_state.blend.b = p_blend_state.blend_constant.b;
		pipeline->raster_state.blend.a = p_blend_state.blend_constant.a;
	}

	// Dynamic state.

	if (p_dynamic_state.has_flag(DYNAMIC_STATE_DEPTH_BIAS)) {
		pipeline->raster_state.depth_bias.enabled = true;
	}

	if (p_dynamic_state.has_flag(DYNAMIC_STATE_BLEND_CONSTANTS)) {
		pipeline->raster_state.blend.enabled = true;
	}

	if (p_dynamic_state.has_flag(DYNAMIC_STATE_DEPTH_BOUNDS)) {
		// TODO(sgc): ??
	}

	if (p_dynamic_state.has_flag(DYNAMIC_STATE_STENCIL_COMPARE_MASK)) {
		// TODO(sgc): ??
	}

	if (p_dynamic_state.has_flag(DYNAMIC_STATE_STENCIL_WRITE_MASK)) {
		// TODO(sgc): ??
	}

	if (p_dynamic_state.has_flag(DYNAMIC_STATE_STENCIL_REFERENCE)) {
		pipeline->raster_state.stencil.enabled = true;
	}

	if (shader->vert != nil) {
		Result<id<MTLFunction>> function_or_err = _create_function(shader->vert, @"main0", p_specialization_constants);
		ERR_FAIL_COND_V(std::holds_alternative<Error>(function_or_err), PipelineID());
		desc.vertexFunction = std::get<id<MTLFunction>>(function_or_err);
	}

	if (shader->frag != nil) {
		Result<id<MTLFunction>> function_or_err = _create_function(shader->frag, @"main0", p_specialization_constants);
		ERR_FAIL_COND_V(std::holds_alternative<Error>(function_or_err), PipelineID());
		desc.fragmentFunction = std::get<id<MTLFunction>>(function_or_err);
	}

	NSError *error = nil;
	pipeline->state = [device newRenderPipelineStateWithDescriptor:desc
															 error:&error];
	pipeline->shader = shader;

	if (error != nil) {
		auto _debug = true;
	}

	ERR_FAIL_COND_V_MSG(error != nil, PipelineID(), ([NSString stringWithFormat:@"error creating pipeline: %@", error.localizedDescription].UTF8String));

	return PipelineID(pipeline);
}

/*****************/
/**** COMPUTE ****/
/*****************/

// ----- COMMANDS -----

void RenderingDeviceDriverMetal::command_bind_compute_pipeline(CommandBufferID p_cmd_buffer, PipelineID p_pipeline) {
	MDCommandBuffer *cb = (MDCommandBuffer *)(p_cmd_buffer.id);
	cb->bind_pipeline(p_pipeline);
}

void RenderingDeviceDriverMetal::command_bind_compute_uniform_set(CommandBufferID p_cmd_buffer, UniformSetID p_uniform_set, ShaderID p_shader, uint32_t p_set_index) {
	MDCommandBuffer *cb = (MDCommandBuffer *)(p_cmd_buffer.id);
	cb->compute_bind_uniform_set(p_uniform_set, p_shader, p_set_index);
}

void RenderingDeviceDriverMetal::command_compute_dispatch(CommandBufferID p_cmd_buffer, uint32_t p_x_groups, uint32_t p_y_groups, uint32_t p_z_groups) {
	MDCommandBuffer *cb = (MDCommandBuffer *)(p_cmd_buffer.id);
	cb->compute_dispatch(p_x_groups, p_y_groups, p_z_groups);
}

void RenderingDeviceDriverMetal::command_compute_dispatch_indirect(CommandBufferID p_cmd_buffer, BufferID p_indirect_buffer, uint64_t p_offset) {
	MDCommandBuffer *cb = (MDCommandBuffer *)(p_cmd_buffer.id);
	cb->compute_dispatch_indirect(p_indirect_buffer, p_offset);
}

// ----- PIPELINE -----

RDD::PipelineID RenderingDeviceDriverMetal::compute_pipeline_create(ShaderID p_shader, VectorView<PipelineSpecializationConstant> p_specialization_constants) {
	MDComputeShader *shader = (MDComputeShader *)(p_shader.id);

	id<MTLLibrary> library = shader->kernel;

	Result<id<MTLFunction>> function_or_err = _create_function(library, @"main0", p_specialization_constants);
	ERR_FAIL_COND_V(std::holds_alternative<Error>(function_or_err), PipelineID());
	id<MTLFunction> function = std::get<id<MTLFunction>>(function_or_err);

	NSError *error;
	id<MTLComputePipelineState> state = [device newComputePipelineStateWithFunction:function error:&error];
	ERR_FAIL_COND_V_MSG(error != nil, PipelineID(), ([NSString stringWithFormat:@"error creating pipeline: %@", error.localizedDescription].UTF8String));

	MDComputePipeline *pipeline = new MDComputePipeline(state);
	pipeline->compute_state.local = shader->local;
	pipeline->shader = shader;

	return PipelineID(pipeline);
}

/*****************/
/**** QUERIES ****/
/*****************/

// ----- TIMESTAMP -----

RDD::QueryPoolID RenderingDeviceDriverMetal::timestamp_query_pool_create(uint32_t p_query_count) {
	NSError *error = nil;
	std::shared_ptr<MDQueryPool> pool = MDQueryPool::new_query_pool(device, &error);
	ERR_FAIL_COND_V_MSG(error != nil, RDD::QueryPoolID(), ([NSString stringWithFormat:@"error creating query pool: %@", error.localizedDescription].UTF8String));
	return rid2::to_id<QueryPoolID>(pool);
}

void RenderingDeviceDriverMetal::timestamp_query_pool_free(QueryPoolID p_pool_id) {
	rid2::release<MDQueryPool>(p_pool_id);
}

void RenderingDeviceDriverMetal::timestamp_query_pool_get_results(QueryPoolID p_pool_id, uint32_t p_query_count, uint64_t *r_results) {
	auto pool = rid2::get<MDQueryPool>(p_pool_id);
	pool->get_results(r_results, p_query_count);
}

uint64_t RenderingDeviceDriverMetal::timestamp_query_result_to_time(uint64_t p_result) {
	return p_result;
}

void RenderingDeviceDriverMetal::command_timestamp_query_pool_reset(CommandBufferID p_cmd_buffer, QueryPoolID p_pool_id, uint32_t p_query_count) {
	auto pool = rid2::get<MDQueryPool>(p_pool_id);
	pool->reset_with_command_buffer(p_cmd_buffer);
}

void RenderingDeviceDriverMetal::command_timestamp_write(CommandBufferID p_cmd_buffer, QueryPoolID p_pool_id, uint32_t p_index) {
	auto pool = rid2::get<MDQueryPool>(p_pool_id);
	pool->write_command_buffer(p_cmd_buffer, p_index);
}

/****************/
/**** SCREEN ****/
/****************/

RDD::DataFormat RenderingDeviceDriverMetal::screen_get_format() {
	return context->get_pixel_formats().getDataFormat(context->get_screen_format());
}

/********************/
/**** SUBMISSION ****/
/********************/

void RenderingDeviceDriverMetal::begin_segment(CommandBufferID p_cmd_buffer, uint32_t p_frame_index, uint32_t p_frames_drawn) {
}

void RenderingDeviceDriverMetal::end_segment() {
}

/**************/
/**** MISC ****/
/**************/

void RenderingDeviceDriverMetal::set_object_name(ObjectType p_type, ID p_driver_id, const String &p_name) {
	switch (p_type) {
		case OBJECT_TYPE_TEXTURE: {
			id<MTLTexture> tex = rid::get(p_driver_id);
			tex.label = [NSString stringWithUTF8String:p_name.utf8().get_data()];
		} break;
		case OBJECT_TYPE_SAMPLER: {
			id<MTLSamplerState> sampler = rid::get(p_driver_id);
			// can't set label after creation
		} break;
		case OBJECT_TYPE_BUFFER: {
			id<MTLBuffer> buffer = rid::get(p_driver_id);
			buffer.label = [NSString stringWithUTF8String:p_name.utf8().get_data()];
		} break;
		case OBJECT_TYPE_SHADER: {
			MDShader *shader = (MDShader *)(p_driver_id.id);
			if (MDRenderShader *rs = dynamic_cast<MDRenderShader *>(shader); rs != nullptr) {
				rs->vert.label = [NSString stringWithUTF8String:p_name.utf8().get_data()];
				rs->frag.label = [NSString stringWithUTF8String:p_name.utf8().get_data()];
			} else if (MDComputeShader *cs = dynamic_cast<MDComputeShader *>(shader); cs != nullptr) {
				cs->kernel.label = [NSString stringWithUTF8String:p_name.utf8().get_data()];
			} else {
				DEV_ASSERT(false);
			}
		} break;
		case OBJECT_TYPE_UNIFORM_SET: {
			MDUniformSet *set = (MDUniformSet *)(p_driver_id.id);
			std::for_each(set->bound_uniforms.begin(), set->bound_uniforms.end(), [&](auto &keyval) {
				keyval.value.buffer.label = [NSString stringWithUTF8String:p_name.utf8().get_data()];
			});
		} break;
		case OBJECT_TYPE_PIPELINE: {
			// can't set label after creation
		} break;
		default: {
			DEV_ASSERT(false);
		}
	}
}

uint64_t RenderingDeviceDriverMetal::get_resource_native_handle(DriverResource p_type, ID p_driver_id) {
	switch (p_type) {
		case DRIVER_RESOURCE_LOGICAL_DEVICE: {
			return 0;
		}
		case DRIVER_RESOURCE_PHYSICAL_DEVICE: {
			return 0;
		}
		case DRIVER_RESOURCE_TOPMOST_OBJECT: {
			return 0;
		}
		case DRIVER_RESOURCE_COMMAND_QUEUE: {
			return 0;
		}
		case DRIVER_RESOURCE_QUEUE_FAMILY: {
			return 0;
		}
		case DRIVER_RESOURCE_TEXTURE: {
			return p_driver_id.id;
		}
		case DRIVER_RESOURCE_TEXTURE_VIEW: {
			return p_driver_id.id;
		}
		case DRIVER_RESOURCE_TEXTURE_DATA_FORMAT: {
			return 0;
		}
		case DRIVER_RESOURCE_SAMPLER: {
			return p_driver_id.id;
		}
		case DRIVER_RESOURCE_UNIFORM_SET:
			return 0;
		case DRIVER_RESOURCE_BUFFER: {
			return p_driver_id.id;
		}
		case DRIVER_RESOURCE_COMPUTE_PIPELINE:
			return 0;
		case DRIVER_RESOURCE_RENDER_PIPELINE:
			return 0;
		default: {
			return 0;
		}
	}
}

uint64_t RenderingDeviceDriverMetal::get_total_memory_used() {
	return device.currentAllocatedSize;
}

uint64_t RenderingDeviceDriverMetal::limit_get(Limit p_limit) {
	MetalDeviceProperties const &props = context->get_device_properties();
	MetalLimits const &limits = props.limits;

#if defined(DEV_ENABLED)
#define UNKNOWN(NAME)                                                            \
	case NAME:                                                                   \
		WARN_PRINT_ONCE("Returning maximum value for unknown limit " #NAME "."); \
		return (uint64_t)1 << 30;
#else
#define UNKNOWN(NAME) \
	case NAME:        \
		return (uint64_t)1 << 30
#endif

	// clang-format off
	switch (p_limit) {
		case LIMIT_MAX_BOUND_UNIFORM_SETS:
			return limits.maxBoundDescriptorSets;
		case LIMIT_MAX_FRAMEBUFFER_COLOR_ATTACHMENTS:
			return limits.maxColorAttachments;
		case LIMIT_MAX_TEXTURES_PER_UNIFORM_SET:
			return limits.maxTexturesPerArgumentBuffer;
		case LIMIT_MAX_SAMPLERS_PER_UNIFORM_SET:
			return limits.maxSamplersPerArgumentBuffer;
		case LIMIT_MAX_STORAGE_BUFFERS_PER_UNIFORM_SET:
			return limits.maxBuffersPerArgumentBuffer;
		case LIMIT_MAX_STORAGE_IMAGES_PER_UNIFORM_SET:
			return limits.maxTexturesPerArgumentBuffer;
		case LIMIT_MAX_UNIFORM_BUFFERS_PER_UNIFORM_SET:
			return limits.maxBuffersPerArgumentBuffer;
		UNKNOWN(LIMIT_MAX_DRAW_INDEXED_INDEX);
		case LIMIT_MAX_FRAMEBUFFER_HEIGHT:
			return limits.maxFramebufferHeight;
		case LIMIT_MAX_FRAMEBUFFER_WIDTH:
			return limits.maxFramebufferWidth;
		case LIMIT_MAX_TEXTURE_ARRAY_LAYERS:
			return limits.maxImageArrayLayers;
		case LIMIT_MAX_TEXTURE_SIZE_1D:
			return limits.maxImageDimension1D;
		case LIMIT_MAX_TEXTURE_SIZE_2D:
			return limits.maxImageDimension2D;
		case LIMIT_MAX_TEXTURE_SIZE_3D:
			return limits.maxImageDimension3D;
		case LIMIT_MAX_TEXTURE_SIZE_CUBE:
			return limits.maxImageDimensionCube;
		case LIMIT_MAX_TEXTURES_PER_SHADER_STAGE:
			return limits.maxTexturesPerArgumentBuffer;
		case LIMIT_MAX_SAMPLERS_PER_SHADER_STAGE:
			return limits.maxSamplersPerArgumentBuffer;
		case LIMIT_MAX_STORAGE_BUFFERS_PER_SHADER_STAGE:
			return limits.maxBuffersPerArgumentBuffer;
		case LIMIT_MAX_STORAGE_IMAGES_PER_SHADER_STAGE:
			return limits.maxTexturesPerArgumentBuffer;
		case LIMIT_MAX_UNIFORM_BUFFERS_PER_SHADER_STAGE:
			return limits.maxBuffersPerArgumentBuffer;
		case LIMIT_MAX_PUSH_CONSTANT_SIZE:
			return limits.maxBufferLength;
		case LIMIT_MAX_UNIFORM_BUFFER_SIZE:
			return limits.maxBufferLength;
		case LIMIT_MAX_VERTEX_INPUT_ATTRIBUTE_OFFSET:
			return limits.maxVertexDescriptorLayoutStride;
		UNKNOWN(LIMIT_MAX_VERTEX_INPUT_ATTRIBUTES);
		UNKNOWN(LIMIT_MAX_VERTEX_INPUT_BINDINGS);
		UNKNOWN(LIMIT_MAX_VERTEX_INPUT_BINDING_STRIDE);
		UNKNOWN(LIMIT_MIN_UNIFORM_BUFFER_OFFSET_ALIGNMENT);
		UNKNOWN(LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_X);
		UNKNOWN(LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_Y);
		UNKNOWN(LIMIT_MAX_COMPUTE_WORKGROUP_COUNT_Z);
		case LIMIT_MAX_COMPUTE_WORKGROUP_INVOCATIONS:
			return std::max({ limits.maxThreadsPerThreadGroup.width, limits.maxThreadsPerThreadGroup.height, limits.maxThreadsPerThreadGroup.depth });
		case LIMIT_MAX_COMPUTE_WORKGROUP_SIZE_X:
			return limits.maxThreadsPerThreadGroup.width;
		case LIMIT_MAX_COMPUTE_WORKGROUP_SIZE_Y:
			return limits.maxThreadsPerThreadGroup.height;
		case LIMIT_MAX_COMPUTE_WORKGROUP_SIZE_Z:
			return limits.maxThreadsPerThreadGroup.depth;
		case LIMIT_MAX_VIEWPORT_DIMENSIONS_X:
			return limits.maxViewportDimensionX;
		case LIMIT_MAX_VIEWPORT_DIMENSIONS_Y:
			return limits.maxViewportDimensionY;
		UNKNOWN(LIMIT_SUBGROUP_SIZE);
		UNKNOWN(LIMIT_SUBGROUP_MIN_SIZE);
		UNKNOWN(LIMIT_SUBGROUP_MAX_SIZE);
		UNKNOWN(LIMIT_SUBGROUP_IN_SHADERS);
		UNKNOWN(LIMIT_SUBGROUP_OPERATIONS);
		UNKNOWN(LIMIT_VRS_TEXEL_WIDTH);
		UNKNOWN(LIMIT_VRS_TEXEL_HEIGHT);
		default:
			ERR_FAIL_V(0);
	}
	// clang-format on
	return 0;
}

uint64_t RenderingDeviceDriverMetal::api_trait_get(ApiTrait p_trait) {
	switch (p_trait) {
		case API_TRAIT_HONORS_PIPELINE_BARRIERS:
			return 0;
		default:
			return RenderingDeviceDriver::api_trait_get(p_trait);
	}
}

bool RenderingDeviceDriverMetal::has_feature(Features p_feature) {
	switch (p_feature) {
		case SUPPORTS_MULTIVIEW:
			return true;
		case SUPPORTS_FSR_HALF_FLOAT:
			return true;
		case SUPPORTS_ATTACHMENT_VRS:
			// TODO(sgc): Maybe supported via https://developer.apple.com/documentation/metal/render_passes/rendering_at_different_rasterization_rates?language=objc
			// See also:
			//
			// * https://forum.beyond3d.com/threads/variable-rate-shading-vs-variable-rate-rasterization.62243/post-2191363
			//
			return false;
		case SUPPORTS_FRAGMENT_SHADER_WITH_ONLY_SIDE_EFFECTS:
			return true;
		default:
			return false;
	}
}

const RDD::MultiviewCapabilities &RenderingDeviceDriverMetal::get_multiview_capabilities() {
	return context->get_multiview_capabilities();
}

/******************/

RenderingDeviceDriverMetal::RenderingDeviceDriverMetal(MetalContext *p_context, id<MTLDevice> p_device) :
		context(p_context), device(p_device) {}
RenderingDeviceDriverMetal::~RenderingDeviceDriverMetal() {
	for (MDCommandBuffer *cb : command_buffers) {
		delete cb;
	}
}