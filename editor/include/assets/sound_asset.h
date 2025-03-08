#pragma once

#include <assets/asset.h>
#include <ermy_sound.h>
#include <vector>

class SoundAsset : public AssetData
{
public:
	AssetDataType GetDataType() override { return AssetDataType::Sound; }

	std::vector<float> samples;
	int channels = 0;
	float volume = 1.0f;
	ermy::sound::SoundID sound;
	double duration = 0.0;

	SoundAsset();
	virtual ~SoundAsset();

	void DrawPreview() override;

	void RegeneratePreview() override;
	void RenderPreview(ermy::rendering::CommandList& cl) override;
	void RenderStaticPreview(ermy::rendering::CommandList& cl) override;

	void MouseZoom(float) override;
	void MouseDown(float normalizedX, float normalizedY, int button) override;
};