#pragma once

#include <assets/asset.h>
#include <ermy_font.h>
#include <vector>
#include <imgui.h>

class FontAsset : public AssetData
{
	void _createLivePreviewTexture();
public:
	AssetDataType GetDataType() override { return AssetDataType::Sound; }

	FontAsset();
	virtual ~FontAsset();

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

	ermy::Font* font = nullptr;
};