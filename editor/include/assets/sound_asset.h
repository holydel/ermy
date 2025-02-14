#pragma once

#include <assets/asset.h>

class SoundAsset : public AssetData
{
public:
	double duration = 0.0;

	SoundAsset();
	virtual ~SoundAsset();

	void DrawPreview() override;
};