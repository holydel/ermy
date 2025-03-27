#pragma once
#include "assets/KTXLoader.h"
#include <ktx.h>
#include "assets/texture_asset.h"
#include <ermy_rendering.h>

using namespace editor::asset::loader;
using namespace ermy;

static std::vector<std::string> gSupportedExtensions;
const std::vector<std::string>& KTXLoader::SupportedExtensions()
{
	gSupportedExtensions.push_back("ktx");
	gSupportedExtensions.push_back("ktx2");
	return gSupportedExtensions;
}

rendering::Format FromGLInternalFormat(u32 glInternalFormat)
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
		return Format::RGBA8_UINT;
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

KTXLoader::KTXLoader()
{

}

KTXLoader::~KTXLoader()
{

}

AssetData* KTXLoader::Load(const std::filesystem::path& path)
{
	ktxTexture* ktxTexture = nullptr;
	ktxTexture_CreateFromNamedFile(path.string().c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);

	if (ktxTexture == nullptr)
		return nullptr;

	TextureAsset* result = new TextureAsset();

	if (ktxTexture->classId == ktxTexture2_c)
	{
		ktxTexture2* ktxTex2 = reinterpret_cast<ktxTexture2*>(ktxTexture);

		result->width = ktxTex2->baseWidth;
		result->height = ktxTex2->baseHeight;
		result->SetSourceData(ktxTex2->pData, ktxTex2->dataSize);
	}

	if (ktxTexture->classId == ktxTexture1_c)
	{
		ktxTexture1* ktxTex1 = reinterpret_cast<ktxTexture1*>(ktxTexture);

		result->width = ktxTex1->baseWidth;
		result->height = ktxTex1->baseHeight;
		
		result->numLayers = ktxTex1->numLayers * ktxTex1->numFaces;
		result->numMips = ktxTex1->numLevels;
		if (ktxTex1->numFaces == 6)
			result->isCubemap = true;

		result->texelSourceFormat = result->texelTargetFormat = FromGLInternalFormat(ktxTex1->glInternalformat);

		result->SetSourceData(ktxTex1->pData, ktxTex1->dataSize);
		//ktxTex1->
		//result->numLayers = ktxTex1->numLay;

	}

	//result->RegenerateLivePreview();

	return result;
}