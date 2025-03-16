#include "ermy_rendering.h"

namespace ermy::rendering {

const char* VertexFormatNames[VERTEX_FORMAT_COUNT] = {
    "UNKNOWN",
    "XYZW_32F",
    "XYZ_32F",
    "XY_32F",
    "X_32F",
    "XYZW_16F",
    "XYZ_16F",
    "XY_16F",
    "X_16F",
    "XYZW_PACKED_32_UNORM",
    "XYZW_PACKED_32_SNORM",
    "XY_PACKED_32_UNORM",
    "XY_PACKED_32_SNORM",
};

const char* FormatNames[FORMAT_COUNT] = {
    "UNKNOWN",
    "RGBA8_UNORM",
    "ARGB8_UNORM",
    "RGBA16_UNORM",
    "RGBA16_NORM",
    "RGBA16F",
    "RG16F",
    "R16F",
    "RGBA16_UINT",
    "RG16_UINT",
    "R16_UINT",
    "RG16_UNORM",
    "R16_UNORM",
    "RGBA8_UINT",
    "R32F",
    "RG32F",
    "RGB32F",
    "RGBA32F",
    "BC1",
    "BC2",
    "BC3",
    "BC4",
    "BC5",
    "BC6",
    "BC6_SF",
    "BC7",
    "R8_UNORM",
    "RG8_UNORM",
    "BGRA8_UNORM",
    "D32F",
    "D16_UNORM",
    "D24_UNORM_S8_UINT",
    "RGBA8_SRGB",
};

} 