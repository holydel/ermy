#pragma once

#include <assets/asset.h>
#include <ermy_sound.h>
#include <vector>

class SoundAsset : public AssetData
{
	void _createLivePreviewTexture();
public:
	AssetDataType GetDataType() override { return AssetDataType::Sound; }

	std::vector<float> samples;
	std::vector<ermy::u8> samplesPreviewRaw;
	int channels = 0;
	float volume = 1.0f;
	ermy::sound::SoundID sound;
	double duration = 0.0;

	SoundAsset();
	virtual ~SoundAsset();

	void DrawPreview() override;

	void RegenerateLivePreview() override;
	void RenderPreview(ermy::rendering::CommandList& cl) override;
	void RenderStaticPreview(ermy::rendering::CommandList& cl) override;
	std::vector<ermy::u8> GetStaticPreviewData() override;

	void MouseZoom(float) override;
	void MouseDown(float normalizedX, float normalizedY, int button) override;

	void Save(pugi::xml_node& node) override;
	void Load(pugi::xml_node& node) override;

	void LoadFromCachedRaw(std::ifstream& file, const std::filesystem::path& path) override;
	void SaveToCachedRaw(std::ofstream& file) override;
};