#pragma once
#include <assets/asset.h>

class VideoTextureAsset : public Asset
{
public:
	struct VideoTextureAssetMeta : public Asset::MetaData
	{

	};

	VideoTextureAsset(const char* filepath);
	virtual ~VideoTextureAsset();

	static std::vector<FormatExtensionInfo> Initialize();
	static void Shutdown();
};