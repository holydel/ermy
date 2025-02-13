#pragma once
#include <assets/asset.h>

class VideoTextureAsset : public AssetData
{
public:
	struct VideoTextureAssetMeta : public AssetData::MetaData
	{

	};

	VideoTextureAsset();
	virtual ~VideoTextureAsset();

};