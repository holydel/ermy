#pragma once

#include <assets/asset.h>

class TextureAsset : public Asset
{
public:
	struct TextureAssetMeta : public Asset::MetaData
	{

	};

	TextureAsset();
	virtual ~TextureAsset();
};