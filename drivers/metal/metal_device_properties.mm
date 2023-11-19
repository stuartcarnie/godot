/**************************************************************************/
/*  metal_device_properties.mm                                            */
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

#include "metal_device_properties.h"

#import "spirv_cross.hpp"
#import "spirv_msl.hpp"
#import <Metal/Metal.h>

// For Apple Silicon, the Device ID is determined by the highest
// GPU capability, which is a combination of OS version and GPU type.
void MetalDeviceProperties::initGpuProperties(id<MTLDevice> device) {
	device_type = RenderingDevice::DEVICE_TYPE_INTEGRATED_GPU;
	device_name = device.name.UTF8String;
}

void MetalDeviceProperties::initFeatures(id<MTLDevice> device) {
	features = { 0 };

	features.hostMemoryPageSize = sysconf(_SC_PAGESIZE);

	for (SampleCount sc = SampleCount1; sc <= SampleCount64; sc <<= 1) {
		if ([device supportsTextureSampleCount:sc]) {
			features.supportedSampleCounts |= sc;
		}
	}

	features.layeredRendering = [device supportsFamily:MTLGPUFamilyApple5];
	features.multisampleLayeredRendering = [device supportsFamily:MTLGPUFamilyApple7];

	features.mslVersionEnum = MTLLanguageVersion1_1;

	if (@available(macOS 11, iOS 14, *)) {
		features.mslVersionEnum = MTLLanguageVersion2_3;
	}
	if (@available(macOS 12, iOS 15, *)) {
		features.mslVersionEnum = MTLLanguageVersion2_4;
	}
	if (@available(macOS 13, iOS 16, *)) {
		features.mslVersionEnum = MTLLanguageVersion3_0;
	}
	if (@available(macOS 14, iOS 17, *)) {
		features.mslVersionEnum = MTLLanguageVersion3_1;
	}

#define setMSLVersion(maj, min) \
	features.mslVersion = SPIRV_CROSS_NAMESPACE::CompilerMSL::Options::make_msl_version(maj, min)

	switch (features.mslVersionEnum) {
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 140000
		case MTLLanguageVersion3_1:
			setMSLVersion(3, 1);
			break;
#endif
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 130000
		case MTLLanguageVersion3_0:
			setMSLVersion(3, 0);
			break;
#endif
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 120000
		case MTLLanguageVersion2_4:
			setMSLVersion(2, 4);
			break;
#endif
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 110000
		case MTLLanguageVersion2_3:
			setMSLVersion(2, 3);
			break;
#endif
		case MTLLanguageVersion2_2:
			setMSLVersion(2, 2);
			break;
		case MTLLanguageVersion2_1:
			setMSLVersion(2, 1);
			break;
		case MTLLanguageVersion2_0:
			setMSLVersion(2, 0);
			break;
		case MTLLanguageVersion1_2:
			setMSLVersion(1, 2);
			break;
		case MTLLanguageVersion1_1:
			setMSLVersion(1, 1);
			break;
#if TARGET_OS_IPHONE
		case MTLLanguageVersion1_0:
			setMSLVersion(1, 0);
			break;
#endif
	}
}

void MetalDeviceProperties::initLimits(id<MTLDevice> device) {
	using std::max;
	using std::min;

	// FST: https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf

	// FST: Maximum number of layers per 1D texture array, 2D texture array, or 3D texture
	limits.maxImageArrayLayers = 2048;
	if ([device supportsFamily:MTLGPUFamilyApple3]) {
		// FST: Maximum 2D texture width and height
		limits.maxFramebufferWidth = 16384;
		limits.maxFramebufferHeight = 16384;
		limits.maxViewportDimensionX = 16384;
		limits.maxViewportDimensionY = 16384;
		// FST: Maximum 1D texture width
		limits.maxImageDimension1D = 16384;
		// FST: Maximum 2D texture width and height
		limits.maxImageDimension2D = 16384;
		// FST: Maximum cube map texture width and height
		limits.maxImageDimensionCube = 16384;
	} else {
		// FST: Maximum 2D texture width and height
		limits.maxFramebufferWidth = 8192;
		limits.maxFramebufferHeight = 8192;
		limits.maxViewportDimensionX = 8192;
		limits.maxViewportDimensionY = 8192;
		// FST: Maximum 1D texture width
		limits.maxImageDimension1D = 8192;
		// FST: Maximum 2D texture width and height
		limits.maxImageDimension2D = 8192;
		// FST: Maximum cube map texture width and height
		limits.maxImageDimensionCube = 8192;
	}
	// FST: Maximum 3D texture width, height, and depth
	limits.maxImageDimension3D = 2048;

	limits.maxThreadsPerThreadGroup = device.maxThreadsPerThreadgroup;
	// https://github.com/KhronosGroup/MoltenVK/blob/568cc3acc0e2299931fdaecaaa1fc3ec5b4af281/MoltenVK/MoltenVK/GPUObjects/MVKDevice.h#L85
	limits.maxBoundDescriptorSets = SPIRV_CROSS_NAMESPACE::kMaxArgumentBuffers;
	// FST: Maximum number of color render targets per render pass descriptor
	limits.maxColorAttachments = 8;

	// Maximum number of textures the device can access, per stage, from an argument buffer
	if ([device supportsFamily:MTLGPUFamilyApple6]) {
		limits.maxTexturesPerArgumentBuffer = 1'000'000;
	} else if ([device supportsFamily:MTLGPUFamilyApple4]) {
		limits.maxTexturesPerArgumentBuffer = 96;
	} else {
		limits.maxTexturesPerArgumentBuffer = 31;
	}

	// Maximum number of samplers the device can access, per stage, from an argument buffer
	if ([device supportsFamily:MTLGPUFamilyApple6]) {
		limits.maxSamplersPerArgumentBuffer = 1024;
	} else {
		limits.maxSamplersPerArgumentBuffer = 16;
	}

	// Maximum number of buffers the device can access, per stage, from an argument buffer
	if ([device supportsFamily:MTLGPUFamilyApple6]) {
		limits.maxBuffersPerArgumentBuffer = std::numeric_limits<uint64_t>::max();
	} else if ([device supportsFamily:MTLGPUFamilyApple4]) {
		limits.maxBuffersPerArgumentBuffer = 96;
	} else {
		limits.maxBuffersPerArgumentBuffer = 31;
	}

	limits.maxBufferLength = device.maxBufferLength;
	// FST: Maximum size of vertex descriptor layout stride
	limits.maxVertexDescriptorLayoutStride = std::numeric_limits<uint64_t>::max();

	// Maxiumum nunmber of viewports
	if ([device supportsFamily:MTLGPUFamilyApple5]) {
		limits.maxViewports = 16;
	} else {
		limits.maxViewports = 1;
	}

	limits.maxPerStageBufferCount = 31;
	limits.maxPerStageSamplerCount = 16;
	if ([device supportsFamily:MTLGPUFamilyApple6]) {
		limits.maxPerStageTextureCount = 128;
	} else if ([device supportsFamily:MTLGPUFamilyApple4]) {
		limits.maxPerStageTextureCount = 96;
	} else {
		limits.maxPerStageTextureCount = 31;
	}
}

void MetalDeviceProperties::initTextureCaps(id<MTLDevice> device) {
}

MetalDeviceProperties::MetalDeviceProperties(id<MTLDevice> device) {
	initGpuProperties(device);
	initFeatures(device);
	initLimits(device);
}

MetalDeviceProperties::~MetalDeviceProperties() {
}

SampleCount MetalDeviceProperties::find_nearest_supported_sample_count(RenderingDevice::TextureSamples samples) const {
	SampleCount supported = features.supportedSampleCounts;
	if (supported & sample_count[samples]) {
		return sample_count[samples];
	}

	SampleCount requested_sample_count = sample_count[samples];
	// Find the nearest supported sample count
	while (requested_sample_count > SampleCount1) {
		if (supported & requested_sample_count) {
			return requested_sample_count;
		}
		requested_sample_count = (SampleCount)(requested_sample_count >> 1);
	}

	return SampleCount1;
}

// region static members

const SampleCount MetalDeviceProperties::sample_count[RenderingDevice::TextureSamples::TEXTURE_SAMPLES_MAX] = {
	SampleCount1,
	SampleCount2,
	SampleCount4,
	SampleCount8,
	SampleCount16,
	SampleCount32,
	SampleCount64,
};

// endregion
