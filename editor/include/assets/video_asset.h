#pragma once
#include <assets/asset.h>

class VideoTextureAsset : public AssetData
{
public:
	AssetDataType GetDataType() override { return AssetDataType::Video; }
	VideoTextureAsset();
	virtual ~VideoTextureAsset();

};