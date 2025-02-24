#pragma once

#include <assets/asset.h>
#include <ermy_sound.h>
#include <vector>

class SoundAsset : public AssetData
{
public:
	std::vector<float> samples;
	int channels = 0;
	ermy::sound::SoundID sound;
	double duration = 0.0;

	SoundAsset();
	virtual ~SoundAsset();

	void DrawPreview() override;

	void RegeneratePreview() override;
	void RenderPreview(ermy::rendering::CommandList& cl) override;
	void RenderStaticPreview(ermy::rendering::CommandList& cl) override;
};