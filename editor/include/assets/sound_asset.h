#pragma once

#include <assets/asset.h>

class SoundAsset : public Asset
{
public:
	struct SoundAssetMeta : public Asset::MetaData
	{

	};

	SoundAsset(const char* filepath);
	virtual ~SoundAsset();

	static std::vector<std::string> Initialize();
	static void Shutdown();
};