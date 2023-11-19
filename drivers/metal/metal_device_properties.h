/**************************************************************************/
/*  metal_device_properties.h                                             */
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

#ifndef METAL_DEVICE_PROPERTIES_H
#define METAL_DEVICE_PROPERTIES_H

#include "core/error/error_list.h"
#include "core/string/ustring.h"
#include "core/templates/hash_map.h"
#include "servers/rendering/rendering_device.h"

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

// Common scaling multipliers
#define KIBI (1024)
#define MEBI (KIBI * KIBI)
#define GIBI (KIBI * MEBI)

/** The buffer index to use for vertex content. */
const static uint32_t VERT_CONTENT_BUFFER_INDEX = 0;
const static uint32_t MAX_COLOR_ATTACHMENT_COUNT = 8;

typedef NS_OPTIONS(NSUInteger, SampleCount) {
	SampleCount1 = (1UL << 0),
	SampleCount2 = (1UL << 1),
	SampleCount4 = (1UL << 2),
	SampleCount8 = (1UL << 3),
	SampleCount16 = (1UL << 4),
	SampleCount32 = (1UL << 5),
	SampleCount64 = (1UL << 6),
};

struct MetalFeatures {
	uint32_t mslVersion;
	MTLGPUFamily highestFamily;
	MTLLanguageVersion mslVersionEnum;
	SampleCount supportedSampleCounts;
	long hostMemoryPageSize;
	bool layeredRendering;
	bool multisampleLayeredRendering;
};

struct MetalLimits {
	uint64_t maxImageArrayLayers;
	uint64_t maxFramebufferHeight;
	uint64_t maxFramebufferWidth;
	uint64_t maxImageDimension1D;
	uint64_t maxImageDimension2D;
	uint64_t maxImageDimension3D;
	uint64_t maxImageDimensionCube;
	uint64_t maxViewportDimensionX;
	uint64_t maxViewportDimensionY;
	MTLSize maxThreadsPerThreadGroup;
	uint64_t maxBoundDescriptorSets;
	uint64_t maxColorAttachments;
	uint64_t maxTexturesPerArgumentBuffer;
	uint64_t maxSamplersPerArgumentBuffer;
	uint64_t maxBuffersPerArgumentBuffer;
	uint64_t maxBufferLength;
	uint64_t maxVertexDescriptorLayoutStride;
	uint16_t maxViewports;
	uint32_t maxPerStageBufferCount; /**< The total number of per-stage Metal buffers available for shader uniform content and attributes. */
	uint32_t maxPerStageTextureCount; /**< The total number of per-stage Metal textures available for shader uniform content. */
	uint32_t maxPerStageSamplerCount; /**< The total number of per-stage Metal samplers available for shader uniform content. */

	bool supportsMultipleViewports() const { return maxViewports > 1; };
};

class MetalDeviceProperties {
private:
	void initGpuProperties(id<MTLDevice> device);
	void initFeatures(id<MTLDevice> device);
	void initLimits(id<MTLDevice> device);
	void initTextureCaps(id<MTLDevice> device);
	bool msl_version_is_at_least(MTLLanguageVersion minVer) { return features.mslVersionEnum >= minVer; }

public:
	RenderingDevice::DeviceType device_type;
	String device_name;
	MetalFeatures features;
	MetalLimits limits;

	SampleCount find_nearest_supported_sample_count(RenderingDevice::TextureSamples samples) const;

	MetalDeviceProperties(id<MTLDevice> device);
	~MetalDeviceProperties();

private:
	static const SampleCount sample_count[RenderingDevice::TextureSamples::TEXTURE_SAMPLES_MAX];
};

#endif //METAL_DEVICE_PROPERTIES_H
