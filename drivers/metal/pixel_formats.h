/**************************************************************************/
/*  pixel_formats.h                                                       */
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

#ifndef PIXELFORMATS_H
#define PIXELFORMATS_H

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#include "metal_context.h"
#include "servers/rendering/rendering_device.h"

#import <Metal/Metal.h>

static const uint32_t _dataFormatCount = 256;
static const uint32_t _dataFormatCoreCount = RenderingDevice::DATA_FORMAT_MAX;
static const uint32_t _mtlPixelFormatCount = 256;
static const uint32_t _mtlPixelFormatCoreCount = MTLPixelFormatX32_Stencil8 + 2; // The actual last enum value is not available on iOS
static const uint32_t _mtlVertexFormatCount = MTLVertexFormatHalf + 1;

#pragma mark -
#pragma mark Metal format capabilities

typedef enum : uint16_t {

	kMVKMTLFmtCapsNone = 0,
	kMVKMTLFmtCapsRead = (1 << 0),
	kMVKMTLFmtCapsFilter = (1 << 1),
	kMVKMTLFmtCapsWrite = (1 << 2),
	kMVKMTLFmtCapsAtomic = (1 << 3),
	kMVKMTLFmtCapsColorAtt = (1 << 4),
	kMVKMTLFmtCapsDSAtt = (1 << 5),
	kMVKMTLFmtCapsBlend = (1 << 6),
	kMVKMTLFmtCapsMSAA = (1 << 7),
	kMVKMTLFmtCapsResolve = (1 << 8),
	kMVKMTLFmtCapsVertex = (1 << 9),

	kMVKMTLFmtCapsRF = (kMVKMTLFmtCapsRead | kMVKMTLFmtCapsFilter),
	kMVKMTLFmtCapsRC = (kMVKMTLFmtCapsRead | kMVKMTLFmtCapsColorAtt),
	kMVKMTLFmtCapsRCB = (kMVKMTLFmtCapsRC | kMVKMTLFmtCapsBlend),
	kMVKMTLFmtCapsRCM = (kMVKMTLFmtCapsRC | kMVKMTLFmtCapsMSAA),
	kMVKMTLFmtCapsRCMB = (kMVKMTLFmtCapsRCM | kMVKMTLFmtCapsBlend),
	kMVKMTLFmtCapsRWC = (kMVKMTLFmtCapsRC | kMVKMTLFmtCapsWrite),
	kMVKMTLFmtCapsRWCB = (kMVKMTLFmtCapsRWC | kMVKMTLFmtCapsBlend),
	kMVKMTLFmtCapsRWCM = (kMVKMTLFmtCapsRWC | kMVKMTLFmtCapsMSAA),
	kMVKMTLFmtCapsRWCMB = (kMVKMTLFmtCapsRWCM | kMVKMTLFmtCapsBlend),
	kMVKMTLFmtCapsRFCMRB = (kMVKMTLFmtCapsRCMB | kMVKMTLFmtCapsFilter | kMVKMTLFmtCapsResolve),
	kMVKMTLFmtCapsRFWCMB = (kMVKMTLFmtCapsRWCMB | kMVKMTLFmtCapsFilter),
	kMVKMTLFmtCapsAll = (kMVKMTLFmtCapsRFWCMB | kMVKMTLFmtCapsResolve),

	kMVKMTLFmtCapsDRM = (kMVKMTLFmtCapsDSAtt | kMVKMTLFmtCapsRead | kMVKMTLFmtCapsMSAA),
	kMVKMTLFmtCapsDRFM = (kMVKMTLFmtCapsDRM | kMVKMTLFmtCapsFilter),
	kMVKMTLFmtCapsDRMR = (kMVKMTLFmtCapsDRM | kMVKMTLFmtCapsResolve),
	kMVKMTLFmtCapsDRFMR = (kMVKMTLFmtCapsDRFM | kMVKMTLFmtCapsResolve),

	kMVKMTLFmtCapsChromaSubsampling = kMVKMTLFmtCapsRF,
	kMVKMTLFmtCapsMultiPlanar = kMVKMTLFmtCapsChromaSubsampling,
} MVKMTLFmtCaps;

inline MVKMTLFmtCaps operator|(MVKMTLFmtCaps leftCaps, MVKMTLFmtCaps rightCaps) {
	return static_cast<MVKMTLFmtCaps>(static_cast<uint32_t>(leftCaps) | rightCaps);
}

inline MVKMTLFmtCaps &operator|=(MVKMTLFmtCaps &leftCaps, MVKMTLFmtCaps rightCaps) {
	return (leftCaps = leftCaps | rightCaps);
}

#pragma mark -
#pragma mark Metal view classes

enum class MVKMTLViewClass : uint8_t {
	None,
	Color8,
	Color16,
	Color32,
	Color64,
	Color128,
	PVRTC_RGB_2BPP,
	PVRTC_RGB_4BPP,
	PVRTC_RGBA_2BPP,
	PVRTC_RGBA_4BPP,
	EAC_R11,
	EAC_RG11,
	EAC_RGBA8,
	ETC2_RGB8,
	ETC2_RGB8A1,
	ASTC_4x4,
	ASTC_5x4,
	ASTC_5x5,
	ASTC_6x5,
	ASTC_6x6,
	ASTC_8x5,
	ASTC_8x6,
	ASTC_8x8,
	ASTC_10x5,
	ASTC_10x6,
	ASTC_10x8,
	ASTC_10x10,
	ASTC_12x10,
	ASTC_12x12,
	BC1_RGBA,
	BC2_RGBA,
	BC3_RGBA,
	BC4_R,
	BC5_RG,
	BC6H_RGB,
	BC7_RGBA,
	Depth24_Stencil8,
	Depth32_Stencil8,
	BGRA10_XR,
	BGR10_XR
};

#pragma mark -
#pragma mark Format descriptors

/** Enumerates the data type of a format. */
typedef enum {
	kMVKFormatNone, /**< Format type is unknown. */
	kMVKFormatColorHalf, /**< A 16-bit floating point color. */
	kMVKFormatColorFloat, /**< A 32-bit floating point color. */
	kMVKFormatColorInt8, /**< A signed 8-bit integer color. */
	kMVKFormatColorUInt8, /**< An unsigned 8-bit integer color. */
	kMVKFormatColorInt16, /**< A signed 16-bit integer color. */
	kMVKFormatColorUInt16, /**< An unsigned 16-bit integer color. */
	kMVKFormatColorInt32, /**< A signed 32-bit integer color. */
	kMVKFormatColorUInt32, /**< An unsigned 32-bit integer color. */
	kMVKFormatDepthStencil, /**< A depth and stencil value. */
	kMVKFormatCompressed, /**< A block-compressed color. */
} MVKFormatType;

typedef struct Extent2D {
	uint32_t width;
	uint32_t height;
} Extent2D;

typedef struct FormatProperties {
	uint32_t linearTilingFeatures;
	uint32_t optimalTilingFeatures;
	uint32_t bufferFeatures;
} FormatProperties;

/** Describes the properties of a DataFormat, including the corresponding Metal pixel and vertex format. */
typedef struct MVKDataFormatDesc {
	RenderingDevice::DataFormat dataFormat;
	MTLPixelFormat mtlPixelFormat;
	MTLPixelFormat mtlPixelFormatSubstitute;
	MTLVertexFormat mtlVertexFormat;
	MTLVertexFormat mtlVertexFormatSubstitute;
	uint8_t chromaSubsamplingPlaneCount;
	uint8_t chromaSubsamplingComponentBits;
	Extent2D blockTexelSize;
	uint32_t bytesPerBlock;
	MVKFormatType formatType;
	FormatProperties properties;
	const char *name;
	bool hasReportedSubstitution;

	inline double bytesPerTexel() const { return (double)bytesPerBlock / (double)(blockTexelSize.width * blockTexelSize.height); };

	inline bool isSupported() const { return (mtlPixelFormat != MTLPixelFormatInvalid || chromaSubsamplingPlaneCount > 1); };
	inline bool isSupportedOrSubstitutable() const { return isSupported() || (mtlPixelFormatSubstitute != MTLPixelFormatInvalid); };

	inline bool vertexIsSupported() const { return (mtlVertexFormat != MTLVertexFormatInvalid); };
	inline bool vertexIsSupportedOrSubstitutable() const { return vertexIsSupported() || (mtlVertexFormatSubstitute != MTLVertexFormatInvalid); };
} MVKDataFormatDesc;

/** Describes the properties of a MTLPixelFormat or MTLVertexFormat. */
typedef struct MVKMTLFormatDesc {
	union {
		MTLPixelFormat mtlPixelFormat;
		MTLVertexFormat mtlVertexFormat;
	};
	RenderingDevice::DataFormat dataFormat;
	MVKMTLFmtCaps mtlFmtCaps;
	MVKMTLViewClass mtlViewClass;
	MTLPixelFormat mtlPixelFormatLinear;
	const char *name;

	inline bool isSupported() const { return (mtlPixelFormat != MTLPixelFormatInvalid) && (mtlFmtCaps != kMVKMTLFmtCapsNone); };
} MVKMTLFormatDesc;

class PixelFormats {
	using RD = RenderingDevice;
	using DataFormat = RenderingDevice::DataFormat;

public:
	/** Returns whether the DataFormat is supported by this implementation. */
	bool isSupported(DataFormat dataFormat);

	/** Returns whether the DataFormat is supported by this implementation, or can be substituted by one that is. */
	bool isSupportedOrSubstitutable(DataFormat dataFormat);

	/** Returns whether the specified Metal MTLPixelFormat can be used as a depth format. */
	bool isDepthFormat(MTLPixelFormat mtlFormat);

	/** Returns whether the specified Metal MTLPixelFormat can be used as a stencil format. */
	bool isStencilFormat(MTLPixelFormat mtlFormat);

	/** Returns whether the specified Metal MTLPixelFormat is a PVRTC format. */
	bool isPVRTCFormat(MTLPixelFormat mtlFormat);

	/** Returns the format type corresponding to the specified Vulkan VkFormat, */
	MVKFormatType getFormatType(DataFormat dataFormat);

	/** Returns the format type corresponding to the specified Metal MTLPixelFormat, */
	MVKFormatType getFormatType(MTLPixelFormat mtlFormat);

	/**
	 * Returns the Metal MTLPixelFormat corresponding to the specified Vulkan VkFormat,
	 * or returns MTLPixelFormatInvalid if no corresponding MTLPixelFormat exists.
	 */
	MTLPixelFormat getMTLPixelFormat(DataFormat datFormat);

	/**
	 * Returns the DataFormat corresponding to the specified Metal MTLPixelFormat,
	 * or returns DATA_FORMAT_MAX if no corresponding VkFormat exists.
	 */
	DataFormat getDataFormat(MTLPixelFormat mtlFormat);

	/**
	 * Returns the size, in bytes, of a texel block of the specified Vulkan format.
	 * For uncompressed formats, the returned value corresponds to the size in bytes of a single texel.
	 */
	uint32_t getBytesPerBlock(DataFormat datFormat);

	/**
	 * Returns the size, in bytes, of a texel block of the specified Metal format.
	 * For uncompressed formats, the returned value corresponds to the size in bytes of a single texel.
	 */
	uint32_t getBytesPerBlock(MTLPixelFormat mtlFormat);

	/** Returns the number of planes of the specified chroma-subsampling (YCbCr) DataFormat */
	uint8_t getChromaSubsamplingPlaneCount(DataFormat dataFormat);

	/** Returns the number of bits per channel of the specified chroma-subsampling (YCbCr) DataFormat */
	uint8_t getChromaSubsamplingComponentBits(DataFormat dataFormat);

	/**
	 * Returns the size, in bytes, of a row of texels of the specified Vulkan format.
	 *
	 * For compressed formats, this takes into consideration the compression block size,
	 * and texelsPerRow should specify the width in texels, not blocks. The result is rounded
	 * up if texelsPerRow is not an integer multiple of the compression block width.
	 */
	size_t getBytesPerRow(DataFormat datFormat, uint32_t texelsPerRow);

	/**
	 * Returns the size, in bytes, of a row of texels of the specified Metal format.
	 *
	 * For compressed formats, this takes into consideration the compression block size,
	 * and texelsPerRow should specify the width in texels, not blocks. The result is rounded
	 * up if texelsPerRow is not an integer multiple of the compression block width.
	 */
	size_t getBytesPerRow(MTLPixelFormat mtlFormat, uint32_t texelsPerRow);

	/**
	 * Returns the size, in bytes, of a texture layer of the specified Vulkan format.
	 *
	 * For compressed formats, this takes into consideration the compression block size,
	 * and texelRowsPerLayer should specify the height in texels, not blocks. The result is
	 * rounded up if texelRowsPerLayer is not an integer multiple of the compression block height.
	 */
	size_t getBytesPerLayer(DataFormat datFormat, size_t bytesPerRow, uint32_t texelRowsPerLayer);

	/**
	 * Returns the size, in bytes, of a texture layer of the specified Metal format.
	 * For compressed formats, this takes into consideration the compression block size,
	 * and texelRowsPerLayer should specify the height in texels, not blocks. The result is
	 * rounded up if texelRowsPerLayer is not an integer multiple of the compression block height.
	 */
	size_t getBytesPerLayer(MTLPixelFormat mtlFormat, size_t bytesPerRow, uint32_t texelRowsPerLayer);

	/**
	 * Returns the Metal MTLVertexFormat corresponding to the specified
	 * DataFormat as used as a vertex attribute format.
	 */
	MTLVertexFormat getMTLVertexFormat(DataFormat dataFormat);
#pragma mark Construction

	explicit PixelFormats(MetalContext *context = nullptr);

protected:
	MetalContext *_context;

	MVKDataFormatDesc &getDataFormatDesc(DataFormat dataFormat);
	MVKDataFormatDesc &getDataFormatDesc(MTLPixelFormat mtlFormat);
	MVKMTLFormatDesc &getMTLPixelFormatDesc(MTLPixelFormat mtlFormat);
	MVKMTLFormatDesc &getMTLVertexFormatDesc(MTLVertexFormat mtlFormat);
	void initVkFormatCapabilities();
	void initMTLPixelFormatCapabilities();
	void initMTLVertexFormatCapabilities();
	void buildMTLFormatMaps();
	void buildVkFormatMaps();
	void setFormatProperties(MVKDataFormatDesc &vkDesc);
	void modifyMTLFormatCapabilities();
	void modifyMTLFormatCapabilities(id<MTLDevice> mtlDevice);
	void addMTLPixelFormatCapabilities(id<MTLDevice> mtlDevice,
			MTLFeatureSet mtlFeatSet,
			MTLPixelFormat mtlPixFmt,
			MVKMTLFmtCaps mtlFmtCaps);
	void addMTLPixelFormatCapabilities(id<MTLDevice> mtlDevice,
			MTLGPUFamily gpuFamily,
			MTLPixelFormat mtlPixFmt,
			MVKMTLFmtCaps mtlFmtCaps);
	void disableMTLPixelFormatCapabilities(MTLPixelFormat mtlPixFmt,
			MVKMTLFmtCaps mtlFmtCaps);
	void disableAllMTLPixelFormatCapabilities(MTLPixelFormat mtlPixFmt);
	void addMTLVertexFormatCapabilities(id<MTLDevice> mtlDevice,
			MTLFeatureSet mtlFeatSet,
			MTLVertexFormat mtlVtxFmt,
			MVKMTLFmtCaps mtlFmtCaps);

	MVKDataFormatDesc _dataFormatDescriptions[_dataFormatCount];
	MVKMTLFormatDesc _mtlPixelFormatDescriptions[_mtlPixelFormatCount];
	MVKMTLFormatDesc _mtlVertexFormatDescriptions[_mtlVertexFormatCount];

	// Vulkan core formats have small values and are mapped by simple lookup array.
	// Vulkan extension formats have larger values and are mapped by a map.
	uint16_t _dataFormatDescIndicesByDataFormatsCore[_dataFormatCoreCount];
	HashMap<uint32_t, uint32_t> _dataFormatDescIndicesByDataFormatsExt;

	// Most Metal formats have small values and are mapped by simple lookup array.
	// Outliers are mapped by a map.
	uint16_t _mtlFormatDescIndicesByMTLPixelFormatsCore[_mtlPixelFormatCoreCount];
	HashMap<uint32_t, uint32_t> _mtlFormatDescIndicesByMTLPixelFormatsExt;

	uint16_t _mtlFormatDescIndicesByMTLVertexFormats[_mtlVertexFormatCount];
};

#pragma clang diagnostic pop

#endif //PIXELFORMATS_H
