#pragma once
#include "assets/KTXLoader.h"
#include <ktx.h>

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
	ktxTexture2* ktxTex2 = nullptr;
	//ktxTexture2_CreateFromNamedFile(path.string().c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture2);

	ktxTexture1* ktxTex1 = nullptr;
	//ktxTexture1_CreateFromNamedFile(path.string().c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture1);

	ktxTexture* ktxTexture = nullptr;
	ktxTexture_CreateFromNamedFile(path.string().c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);

	if (ktxTexture->classId == ktxTexture1_c)
	{

	}

	if (ktxTexture->classId == ktxTexture2_c)
	{
		ktxTex2 = reinterpret_cast<ktxTexture2*>(ktxTexture);
	}

	if (ktxTexture->classId == ktxTexture1_c)
	{
		ktxTex1 = reinterpret_cast<ktxTexture1*>(ktxTexture);
	}

	return nullptr;
}