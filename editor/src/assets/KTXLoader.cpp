#pragma once
#include "assets/KTXLoader.h"
#include <ktx.h>
#include "assets/texture_asset.h"


using namespace editor::asset::loader;

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

	if (ktxTexture->classId == ktxTexture2_c)
	{
		ktxTexture2* ktxTex2 = reinterpret_cast<ktxTexture2*>(ktxTexture);

		result->width = ktxTex2->baseWidth;
		result->height = ktxTex2->baseHeight;
		result->data = ktxTex2->pData;
		result->dataSize = ktxTex2->dataSize;
	}

	if (ktxTexture->classId == ktxTexture1_c)
	{
		ktxTexture1* ktxTex1 = reinterpret_cast<ktxTexture1*>(ktxTexture);

		result->width = ktxTex1->baseWidth;
		result->height = ktxTex1->baseHeight;
		result->data = ktxTex1->pData;
		result->dataSize = ktxTex1->dataSize;
	}

	result->RegeneratePreview();

	return result;
}