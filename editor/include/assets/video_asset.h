#pragma once
#include <assets/asset.h>

class VideoTextureAsset : public Asset
{
public:
	struct VideoTextureAssetMeta : public Asset::MetaData
	{

	};

	VideoTextureAsset();
	virtual ~VideoTextureAsset();

};