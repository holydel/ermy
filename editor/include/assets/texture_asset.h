#pragma once

#include <assets/asset.h>

class TextureAsset : public AssetData
{
public:
	ermy::u32 width = 0;
	ermy::u32 height = 0; 
	ermy::u32 depth = 1;
	ermy::u32 numLayers = 1;
	ermy::u32 numMips = 1;
	ermy::u32 dataSize = 1;
	ermy::u8 numChannels = 4;
	ermy::u8 channelBytes = 1;
	bool isCubemap = 0;
	bool isSparse = 0;
	void* data = nullptr;

	TextureAsset();
	virtual ~TextureAsset();

	void DrawPreview() override;

	void RegeneratePreview() override;
	void RenderPreview(ermy::rendering::CommandList& cl) override;
};