#include "ermy_rendering.h"
#include <cassert>

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

Format GetFormatFromName(const char* name)
{
	for (u32 i = 0; i < FORMAT_COUNT; i++)
	{
		if (strcmp(name, FormatNames[i]) == 0)
			return (Format)i;
	}
	return Format::UNKNOWN;
}

rendering::Format GetFormatFromGLInternalFormat(u32 glInternalFormat)
{
	using namespace rendering;

	constexpr u32 ALPHA4_EXT = 0x803B;
	constexpr u32 ALPHA8_EXT = 0x803C;
	constexpr u32 ALPHA12_EXT = 0x803D;
	constexpr u32 ALPHA16_EXT = 0x803E;
	constexpr u32 LUMINANCE4_EXT = 0x803F;
	constexpr u32 LUMINANCE8_EXT = 0x8040;
	constexpr u32 LUMINANCE12_EXT = 0x8041;
	constexpr u32 LUMINANCE16_EXT = 0x8042;
	constexpr u32 LUMINANCE4_ALPHA4_EXT = 0x8043;
	constexpr u32 LUMINANCE6_ALPHA2_EXT = 0x8044;
	constexpr u32 LUMINANCE8_ALPHA8_EXT = 0x8045;
	constexpr u32 LUMINANCE12_ALPHA4_EXT = 0x8046;
	constexpr u32 LUMINANCE12_ALPHA12_EXT = 0x8047;
	constexpr u32 LUMINANCE16_ALPHA16_EXT = 0x8048;
	constexpr u32 INTENSITY_EXT = 0x8049;
	constexpr u32 INTENSITY4_EXT = 0x804A;
	constexpr u32 INTENSITY8_EXT = 0x804B;
	constexpr u32 INTENSITY12_EXT = 0x804C;
	constexpr u32 INTENSITY16_EXT = 0x804D;
	constexpr u32 RGB2_EXT = 0x804E;
	constexpr u32 RGB4_EXT = 0x804F;
	constexpr u32 RGB5_EXT = 0x8050;
	constexpr u32 RGB8_EXT = 0x8051;
	constexpr u32 RGB10_EXT = 0x8052;
	constexpr u32 RGB12_EXT = 0x8053;
	constexpr u32 RGB16_EXT = 0x8054;
	constexpr u32 RGBA2_EXT = 0x8055;
	constexpr u32 RGBA4_EXT = 0x8056;
	constexpr u32 RGB5_A1_EXT = 0x8057;
	constexpr u32 RGBA8_EXT = 0x8058;
	constexpr u32 RGB10_A2_EXT = 0x8059;
	constexpr u32 RGBA12_EXT = 0x805A;;
	constexpr u32 RGBA16_EXT = 0x805B;
	constexpr u32 RGBA32F_EXT = 0x8814;  /* reuse tokens from ARB_texture_float */
	constexpr u32 RGB32F_EXT = 0x8815;
	constexpr u32 ALPHA32F_EXT = 0x8816;
	constexpr u32 LUMINANCE32F_EXT = 0x8818;
	constexpr u32 LUMINANCE_ALPHA32F_EXT = 0x8819;
	constexpr u32 RGBA16F_EXT = 0x881A;  /* reuse tokens from ARB_texture_float */
	constexpr u32 RGB16F_EXT = 0x881B;
	constexpr u32 ALPHA16F_EXT = 0x881C;
	constexpr u32 LUMINANCE16F_EXT = 0x881E;
	constexpr u32 LUMINANCE_ALPHA16F_EXT = 0x881F;
	constexpr u32 BGRA8_EXT = 0x93A1;
	constexpr u32 R8_EXT = 0x8229;  /* reuse tokens from ARB_texture_rg */
	constexpr u32 RG8_EXT = 0x822B;
	constexpr u32 R32F_EXT = 0x822E;  /* reuse tokens from ARB_texture_rg */
	constexpr u32 RG32F_EXT = 0x8230;
	constexpr u32 R16F_EXT = 0x822D;  /* reuse tokens from ARB_texture_g */
	constexpr u32 RG16F_EXT = 0x822F;
	constexpr u32 RGB_RAW_422_APPLE = 0x8A51;
	constexpr u32 GL_R16UI = 0x8234;
	constexpr u32 GL_RGBA8UI_EXT = 0x8D7C;

	constexpr u32 GL_COMPRESSED_RGBA_ASTC_4x4_KHR = 0x93B0;
	constexpr u32 GL_COMPRESSED_RGBA_ASTC_5x4_KHR = 0x93B1;
	constexpr u32 GL_COMPRESSED_RGBA_ASTC_5x5_KHR = 0x93B2;
	constexpr u32 GL_COMPRESSED_RGBA_ASTC_6x5_KHR = 0x93B3;
	constexpr u32 GL_COMPRESSED_RGBA_ASTC_6x6_KHR = 0x93B4;
	constexpr u32 GL_COMPRESSED_RGBA_ASTC_8x5_KHR = 0x93B5;
	constexpr u32 GL_COMPRESSED_RGBA_ASTC_8x6_KHR = 0x93B6;
	constexpr u32 GL_COMPRESSED_RGBA_ASTC_8x8_KHR = 0x93B7;
	constexpr u32 GL_COMPRESSED_RGBA_ASTC_10x5_KHR = 0x93B8;
	constexpr u32 GL_COMPRESSED_RGBA_ASTC_10x6_KHR = 0x93B9;
	constexpr u32 GL_COMPRESSED_RGBA_ASTC_10x8_KHR = 0x93BA;
	constexpr u32 GL_COMPRESSED_RGBA_ASTC_10x10_KHR = 0x93BB;
	constexpr u32 GL_COMPRESSED_RGBA_ASTC_12x10_KHR = 0x93BC;
	constexpr u32 GL_COMPRESSED_RGBA_ASTC_12x12_KHR = 0x93BD;
	constexpr u32 GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR = 0x93D0;
	constexpr u32 GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR = 0x93D1;
	constexpr u32 GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR = 0x93D2;
	constexpr u32 GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR = 0x93D3;
	constexpr u32 GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR = 0x93D4;
	constexpr u32 GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR = 0x93D5;
	constexpr u32 GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR = 0x93D6;
	constexpr u32 GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR = 0x93D7;
	constexpr u32 GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR = 0x93D8;
	constexpr u32 GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR = 0x93D9;
	constexpr u32 GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR = 0x93DA;
	constexpr u32 GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR = 0x93DB;
	constexpr u32 GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR = 0x93DC;
	constexpr u32 GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR = 0x93DD;
	constexpr u32 GL_R16 = 0x822A;
	constexpr u32 GL_SRGB8_ALPHA8_EXT = 0x8C43;
	switch (glInternalFormat)
	{
	case RGBA8_EXT:
		return Format::RGBA8_UNORM;
		//case ARGB8_EXT:
		//	return Format::ARGB8_UNORM;
	case RGBA16F_EXT:
		return Format::RGBA16F;
	case RG16F_EXT:
		return Format::RG16F;
	case R16F_EXT:
		return Format::R16F;
	case GL_R16UI:
		return Format::R16_UINT;
	case GL_RGBA8UI_EXT:
		return Format::RGBA8_UNORM; //RGBA8_UINT
	case GL_R16:
		return Format::R16_UNORM;
	case GL_SRGB8_ALPHA8_EXT:
		return Format::RGBA8_SRGB;
	case R8_EXT:
		return Format::R8_UNORM;
	case RG8_EXT:
		return Format::RG8_UNORM;
	default:
		assert(!"Unknown format");
	}

	return Format::RGBA8_UNORM;
	//	case GL_RGBA8_:

}

rendering::Format GetFormatFromVKFormat(u32 vk_format)
{
    using namespace rendering;

    // Vulkan format constants from vulkan_core.h
    constexpr u64 VK_FORMAT_R8_UNORM = 9;
    constexpr u64 VK_FORMAT_R8G8_UNORM = 16;
    constexpr u64 VK_FORMAT_R8G8B8A8_UNORM = 37;
    constexpr u64 VK_FORMAT_R8G8B8A8_UINT = 41;
    constexpr u64 VK_FORMAT_R8G8B8A8_SRGB = 43;
    constexpr u64 VK_FORMAT_B8G8R8A8_UNORM = 44;
    constexpr u64 VK_FORMAT_R16_UNORM = 73;
    constexpr u64 VK_FORMAT_R16G16_UNORM = 75;
    constexpr u64 VK_FORMAT_R16_SFLOAT = 76;
    constexpr u64 VK_FORMAT_R16_UINT = 77;
    constexpr u64 VK_FORMAT_R16G16_UINT = 79;
    constexpr u64 VK_FORMAT_R16G16B16A16_UNORM = 91;
    constexpr u64 VK_FORMAT_R16G16B16A16_UINT = 93;
    constexpr u64 VK_FORMAT_R16G16_SFLOAT = 94;
    constexpr u64 VK_FORMAT_R16G16B16A16_SFLOAT = 97;
    constexpr u64 VK_FORMAT_R32_SFLOAT = 98;
    constexpr u64 VK_FORMAT_R32G32_SFLOAT = 100;
    constexpr u64 VK_FORMAT_R32G32B32_SFLOAT = 102;
    constexpr u64 VK_FORMAT_R32G32B32A32_SFLOAT = 104;
    constexpr u64 VK_FORMAT_D16_UNORM = 124;
    constexpr u64 VK_FORMAT_D32_SFLOAT = 126;
    constexpr u64 VK_FORMAT_D24_UNORM_S8_UINT = 129;
    constexpr u64 VK_FORMAT_BC1_RGB_UNORM_BLOCK = 131;
    constexpr u64 VK_FORMAT_BC1_RGBA_UNORM_BLOCK = 132;
    constexpr u64 VK_FORMAT_BC2_UNORM_BLOCK = 133;
    constexpr u64 VK_FORMAT_BC3_UNORM_BLOCK = 134;
    constexpr u64 VK_FORMAT_BC4_UNORM_BLOCK = 135;
    constexpr u64 VK_FORMAT_BC5_UNORM_BLOCK = 137;
    constexpr u64 VK_FORMAT_BC6H_UFLOAT_BLOCK = 138;
    constexpr u64 VK_FORMAT_BC6H_SFLOAT_BLOCK = 139;
    constexpr u64 VK_FORMAT_BC7_UNORM_BLOCK = 140;

    switch (vk_format)
    {
        // 8-bit formats
    case VK_FORMAT_R8_UNORM:            return Format::R8_UNORM;
    case VK_FORMAT_R8G8_UNORM:          return Format::RG8_UNORM;
    case VK_FORMAT_R8G8B8A8_UNORM:      return Format::RGBA8_UNORM;
    case VK_FORMAT_B8G8R8A8_UNORM:      return Format::BGRA8_UNORM;
    case VK_FORMAT_R8G8B8A8_SRGB:       return Format::RGBA8_SRGB;

        // 16-bit formats
    case VK_FORMAT_R16_UNORM:           return Format::R16_UNORM;
    case VK_FORMAT_R16G16_UNORM:        return Format::RG16_UNORM;
    case VK_FORMAT_R16G16B16A16_UNORM:  return Format::RGBA16_UNORM;

        // Floating point formats
    case VK_FORMAT_R16_SFLOAT:          return Format::R16F;
    case VK_FORMAT_R16G16_SFLOAT:       return Format::RG16F;
    case VK_FORMAT_R16G16B16A16_SFLOAT: return Format::RGBA16F;
    case VK_FORMAT_R32_SFLOAT:          return Format::R32F;
    case VK_FORMAT_R32G32_SFLOAT:       return Format::RG32F;
    case VK_FORMAT_R32G32B32_SFLOAT:    return Format::RGB32F;
    case VK_FORMAT_R32G32B32A32_SFLOAT: return Format::RGBA32F;

        // Unsigned integer formats
    case VK_FORMAT_R8G8B8A8_UINT:       return Format::RGBA8_UINT;
    case VK_FORMAT_R16_UINT:            return Format::R16_UINT;
    case VK_FORMAT_R16G16_UINT:         return Format::RG16_UINT;
    case VK_FORMAT_R16G16B16A16_UINT:   return Format::RGBA16_UINT;

        // Depth/stencil formats
    case VK_FORMAT_D16_UNORM:           return Format::D16_UNORM;
    case VK_FORMAT_D32_SFLOAT:          return Format::D32F;
    case VK_FORMAT_D24_UNORM_S8_UINT:   return Format::D24_UNORM_S8_UINT;

        // Block compressed formats
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK: return Format::BC1;
    case VK_FORMAT_BC2_UNORM_BLOCK:      return Format::BC2;
    case VK_FORMAT_BC3_UNORM_BLOCK:      return Format::BC3;
    case VK_FORMAT_BC4_UNORM_BLOCK:      return Format::BC4;
    case VK_FORMAT_BC5_UNORM_BLOCK:      return Format::BC5;
    case VK_FORMAT_BC6H_UFLOAT_BLOCK:    return Format::BC6;
    case VK_FORMAT_BC6H_SFLOAT_BLOCK:    return Format::BC6_SF;
    case VK_FORMAT_BC7_UNORM_BLOCK:      return Format::BC7;

    default:
        assert(!"Unsupported Vulkan format conversion");
    }

    return Format::RGBA8_UNORM;
}

rendering::Format GetFormatFromDXGIFormat(u32 dxgi_format)
{
    using namespace rendering;

    // DXGI_FORMAT enum values from dxgiformat.h
    constexpr u32 DXGI_FORMAT_R8_UNORM = 61;
    constexpr u32 DXGI_FORMAT_R8G8_UNORM = 49;
    constexpr u32 DXGI_FORMAT_R8G8B8A8_UNORM = 28;
    constexpr u32 DXGI_FORMAT_B8G8R8A8_UNORM = 87;
    constexpr u32 DXGI_FORMAT_R16_UNORM = 56;
    constexpr u32 DXGI_FORMAT_R16G16_UNORM = 35;
    constexpr u32 DXGI_FORMAT_R16G16B16A16_UNORM = 11;
    constexpr u32 DXGI_FORMAT_R16_FLOAT = 54;
    constexpr u32 DXGI_FORMAT_R16G16_FLOAT = 34;
    constexpr u32 DXGI_FORMAT_R16G16B16A16_FLOAT = 10;
    constexpr u32 DXGI_FORMAT_R32_FLOAT = 41;
    constexpr u32 DXGI_FORMAT_R32G32_FLOAT = 16;
    constexpr u32 DXGI_FORMAT_R32G32B32_FLOAT = 6;
    constexpr u32 DXGI_FORMAT_R32G32B32A32_FLOAT = 2;
    constexpr u32 DXGI_FORMAT_R8G8B8A8_UINT = 30;
    constexpr u32 DXGI_FORMAT_R16_UINT = 57;
    constexpr u32 DXGI_FORMAT_R16G16_UINT = 36;
    constexpr u32 DXGI_FORMAT_R16G16B16A16_UINT = 12;
    constexpr u32 DXGI_FORMAT_D16_UNORM = 55;
    constexpr u32 DXGI_FORMAT_D32_FLOAT = 40;
    constexpr u32 DXGI_FORMAT_D24_UNORM_S8_UINT = 45;
    constexpr u32 DXGI_FORMAT_BC1_UNORM = 71;
    constexpr u32 DXGI_FORMAT_BC2_UNORM = 74;
    constexpr u32 DXGI_FORMAT_BC3_UNORM = 77;
    constexpr u32 DXGI_FORMAT_BC4_UNORM = 80;
    constexpr u32 DXGI_FORMAT_BC5_UNORM = 83;
    constexpr u32 DXGI_FORMAT_BC6H_UF16 = 95;
    constexpr u32 DXGI_FORMAT_BC6H_SF16 = 96;
    constexpr u32 DXGI_FORMAT_BC7_UNORM = 98;
    constexpr u32 DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29;
    constexpr u32 DXGI_FORMAT_B4G4R4A4_UNORM = 88;

    switch (dxgi_format)
    {
        // 8-bit formats
    case DXGI_FORMAT_R8_UNORM:             return Format::R8_UNORM;
    case DXGI_FORMAT_R8G8_UNORM:           return Format::RG8_UNORM;
    case DXGI_FORMAT_R8G8B8A8_UNORM:       return Format::RGBA8_UNORM;
    case DXGI_FORMAT_B8G8R8A8_UNORM:       return Format::BGRA8_UNORM;
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:  return Format::RGBA8_SRGB;

        // 16-bit formats
    case DXGI_FORMAT_R16_UNORM:            return Format::R16_UNORM;
    case DXGI_FORMAT_R16G16_UNORM:         return Format::RG16_UNORM;
    case DXGI_FORMAT_R16G16B16A16_UNORM:   return Format::RGBA16_UNORM;
    case DXGI_FORMAT_B4G4R4A4_UNORM:       return Format::RG8_UNORM;
        // Floating point formats
    case DXGI_FORMAT_R16_FLOAT:            return Format::R16F;
    case DXGI_FORMAT_R16G16_FLOAT:         return Format::RG16F;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:   return Format::RGBA16F;
    case DXGI_FORMAT_R32_FLOAT:            return Format::R32F;
    case DXGI_FORMAT_R32G32_FLOAT:         return Format::RG32F;
    case DXGI_FORMAT_R32G32B32_FLOAT:      return Format::RGB32F;
    case DXGI_FORMAT_R32G32B32A32_FLOAT:   return Format::RGBA32F;

        // Unsigned integer formats
    case DXGI_FORMAT_R8G8B8A8_UINT:        return Format::RGBA8_UINT;
    case DXGI_FORMAT_R16_UINT:             return Format::R16_UINT;
    case DXGI_FORMAT_R16G16_UINT:          return Format::RG16_UINT;
    case DXGI_FORMAT_R16G16B16A16_UINT:    return Format::RGBA16_UINT;

        // Depth formats
    case DXGI_FORMAT_D16_UNORM:            return Format::D16_UNORM;
    case DXGI_FORMAT_D32_FLOAT:            return Format::D32F;
    case DXGI_FORMAT_D24_UNORM_S8_UINT:    return Format::D24_UNORM_S8_UINT;

        // Block compressed formats
    case DXGI_FORMAT_BC1_UNORM:            return Format::BC1;
    case DXGI_FORMAT_BC2_UNORM:            return Format::BC2;
    case DXGI_FORMAT_BC3_UNORM:            return Format::BC3;
    case DXGI_FORMAT_BC4_UNORM:            return Format::BC4;
    case DXGI_FORMAT_BC5_UNORM:            return Format::BC5;
    case DXGI_FORMAT_BC6H_UF16:            return Format::BC6;
    case DXGI_FORMAT_BC6H_SF16:            return Format::BC6_SF;
    case DXGI_FORMAT_BC7_UNORM:            return Format::BC7;

    default:
        assert(!"Unsupported DXGI format conversion");
    }

    return Format::RGBA8_UNORM;
}

} 