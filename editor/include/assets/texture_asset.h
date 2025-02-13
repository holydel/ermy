#pragma once

#include <assets/asset.h>

class TextureAsset : public AssetData
{
public:
	struct TextureAssetMeta : public AssetData::MetaData
	{

	};

	TextureAsset();
	virtual ~TextureAsset();

	void DrawProps() override;
};