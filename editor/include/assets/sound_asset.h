#pragma once

#include <assets/asset.h>

class SoundAsset : public Asset
{
public:
	struct SoundAssetMeta : public Asset::MetaData
	{

	};

	SoundAsset();
	virtual ~SoundAsset();
};