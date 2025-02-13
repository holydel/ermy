#pragma once

#include <assets/asset.h>

class SoundAsset : public AssetData
{
public:
	struct SoundAssetMeta : public AssetData::MetaData
	{

	};

	SoundAsset();
	virtual ~SoundAsset();
};