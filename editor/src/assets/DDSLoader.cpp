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

AssetData* DDSLoader::Load(const std::filesystem::path& path)
{
	TexMetadata metadata;
	ScratchImage scratchImg;

	// Load the DDS file
	HRESULT hr = LoadFromDDSFile(path.c_str(), DDS_FLAGS_ALLOW_LARGE_FILES, &metadata, scratchImg);
	if (FAILED(hr))
		return nullptr;

	TextureAsset* result = new TextureAsset();
	result->texelSourceFormat = result->texelTargetFormat = ermy::rendering::GetFormatFromDXGIFormat(metadata.format);
	auto finfo = rendering::GetFormatInfo(result->texelSourceFormat);

	int texDataSize = scratchImg.GetPixelsSize();

	result->width = metadata.width;
	result->height = metadata.height;
	result->depth = metadata.depth;
	result->isCubemap = metadata.IsCubemap();
	result->numLayers = metadata.arraySize;
	result->isSparse = false;
	result->numMips = metadata.mipLevels;

	result->SetSourceData(scratchImg.GetPixels(), texDataSize);

	//result->data = malloc(texDataSize);
	//memcpy(result->data, scratchImg.GetPixels(), texDataSize);
	//result->dataSize = texDataSize;

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

	//	result->texelSourceFormat = FromGLInternalFormat(ktxTex1->glInternalformat);

	//	//ktxTex1->
	//	//result->numLayers = ktxTex1->numLay;

	//}

	//result->RegenerateLivePreview();

	return result;
}