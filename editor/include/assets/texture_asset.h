#pragma once

#include <assets/asset.h>

class TextureAsset : public Asset
{
public:
	struct TextureAssetMeta : public Asset::MetaData
	{

	};

	TextureAsset(const char* filepath);
	virtual ~TextureAsset();

	static std::vector<std::string> Initialize();
	static void Shutdown();
};