#pragma once
#include "assets/DDSLoader.h"
#include <DirectXTex.h>
#include "assets/texture_asset.h"
#include <ermy_rendering.h>

using namespace editor::asset::loader;
using namespace ermy;
using namespace ermy::rendering;
using namespace  DirectX;

static std::vector<std::string> gSupportedExtensions;
const std::vector<std::string>& DDSLoader::SupportedExtensions()
{
	gSupportedExtensions.push_back("dds");
	return gSupportedExtensions;
}

DDSLoader::DDSLoader()
{

}

DDSLoader::~DDSLoader()
{

}

Format GetFormatFromDXGI(DXGI_FORMAT dxgiFormat)
{
	switch (dxgiFormat)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM:        return Format::RGBA8_UNORM;
	case DXGI_FORMAT_R16G16B16A16_UNORM:    return Format::RGBA16_UNORM;
	case DXGI_FORMAT_R16G16B16A16_SNORM:    return Format::RGBA16_NORM;
	case DXGI_FORMAT_R16G16B16A16_FLOAT:    return Format::RGBA16F;
	case DXGI_FORMAT_R16G16B16A16_UINT:     return Format::RGBA16_UINT;
	case DXGI_FORMAT_R16G16_UINT:           return Format::RG16_UINT;
	case DXGI_FORMAT_R16_UINT:              return Format::R16_UINT;
	case DXGI_FORMAT_R16G16_UNORM:          return Format::RG16_UNORM;
	case DXGI_FORMAT_R16_UNORM:             return Format::R16_UNORM;
	case DXGI_FORMAT_R8G8B8A8_UINT:         return Format::RGBA8_UINT;
	case DXGI_FORMAT_R32_FLOAT:             return Format::R32F;
	case DXGI_FORMAT_R32G32_FLOAT:          return Format::RG32F;
	case DXGI_FORMAT_R32G32B32_FLOAT:       return Format::RGB32F;
	case DXGI_FORMAT_R32G32B32A32_FLOAT:    return Format::RGBA32F;
	case DXGI_FORMAT_BC1_UNORM:             return Format::BC1;
	case DXGI_FORMAT_BC2_UNORM:             return Format::BC2;
	case DXGI_FORMAT_BC3_UNORM:             return Format::BC3;
	case DXGI_FORMAT_BC4_UNORM:             return Format::BC4;
	case DXGI_FORMAT_BC5_UNORM:             return Format::BC5;
	case DXGI_FORMAT_BC6H_UF16:             return Format::BC6;
	case DXGI_FORMAT_BC7_UNORM:             return Format::BC7;
	case DXGI_FORMAT_R8_UNORM:				return Format::R8_UNORM;
	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM:		return Format::BGRA8_UNORM;
	default:
		assert(!"add  missed format");
	}

	return Format::RGBA8_UNORM;
}

AssetData* DDSLoader::Load(const std::filesystem::path& path)
{
	TexMetadata metadata;
	ScratchImage scratchImg;

	// Load the DDS file
	HRESULT hr = LoadFromDDSFile(path.c_str(), DDS_FLAGS_ALLOW_LARGE_FILES, &metadata, scratchImg);
	if (FAILED(hr))
		return nullptr;

	TextureAsset* result = new TextureAsset();
	result->texelFormat = GetFormatFromDXGI(metadata.format);
	auto finfo = rendering::GetFormatInfo(result->texelFormat);

	int texDataSize = scratchImg.GetPixelsSize();
	result->data = malloc(texDataSize);
	memcpy(result->data, scratchImg.GetPixels(), texDataSize);
	result->dataSize = texDataSize;

	result->width = metadata.width;
	result->height = metadata.height;
	result->depth = metadata.depth;
	result->isCubemap = metadata.IsCubemap();
	result->numLayers = metadata.arraySize;
	result->isSparse = false;
	result->numMips = metadata.mipLevels;

	//for (int i = 0; i < metadata.arraySize; ++i)
	//{
	//	for (int j = 0; j < metadata.mipLevels; ++j)
	//	{
	//		
	//		//scratchImg.
	//	}
	//}
	//printf("foo");
	//if (ktxTexture->classId == ktxTexture2_c)
	//{
	//	ktxTexture2* ktxTex2 = reinterpret_cast<ktxTexture2*>(ktxTexture);

	//	result->width = ktxTex2->baseWidth;
	//	result->height = ktxTex2->baseHeight;
	//	result->data = ktxTex2->pData;
	//	result->dataSize = static_cast<u32>(ktxTex2->dataSize);
	//}

	//if (ktxTexture->classId == ktxTexture1_c)
	//{
	//	ktxTexture1* ktxTex1 = reinterpret_cast<ktxTexture1*>(ktxTexture);

	//	result->width = ktxTex1->baseWidth;
	//	result->height = ktxTex1->baseHeight;
	//	result->data = ktxTex1->pData;
	//	result->dataSize = static_cast<u32>(ktxTex1->dataSize);
	//	result->numLayers = ktxTex1->numLayers * ktxTex1->numFaces;
	//	if (ktxTex1->numFaces == 6)
	//		result->isCubemap = true;

	//	result->texelFormat = FromGLInternalFormat(ktxTex1->glInternalformat);

	//	//ktxTex1->
	//	//result->numLayers = ktxTex1->numLay;

	//}

	result->RegeneratePreview();

	return result;
}