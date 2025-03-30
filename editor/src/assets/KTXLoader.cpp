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

	result->width = ktxTexture->baseWidth;
	result->height = ktxTexture->baseHeight;
	result->depth = ktxTexture->baseDepth;
	result->numLayers = ktxTexture->numLayers * ktxTexture->numFaces;
	result->numMips = ktxTexture->numLevels;

	if (ktxTexture->numFaces == 6)
		result->isCubemap = true;

	if (ktxTexture->classId == ktxTexture2_c)
	{
		ktxTexture2* ktxTex2 = reinterpret_cast<ktxTexture2*>(ktxTexture);
		result->texelSourceFormat = result->texelTargetFormat = ermy::rendering::GetFormatFromVKFormat(ktxTex2->vkFormat);
	}

	if (ktxTexture->classId == ktxTexture1_c)
	{
		ktxTexture1* ktxTex1 = reinterpret_cast<ktxTexture1*>(ktxTexture);
		result->texelSourceFormat = result->texelTargetFormat = ermy::rendering::GetFormatFromGLInternalFormat(ktxTex1->glInternalformat);
	}

	result->SetSourceData(ktxTexture->pData, ktxTexture->dataSize);
	//result->RegenerateLivePreview();

	return result;
}