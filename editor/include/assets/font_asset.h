#pragma once

#include <assets/asset.h>
#include <ermy_font.h>
#include <vector>
#include <imgui.h>

class FontAsset : public AssetData
{
	void _createLivePreviewTexture();
	int numberOfCharacters = 0;

public:
	struct FontAtlas
	{
		ermy::Font::AtlasType type = ermy::Font::AtlasType::Grayscale;
		ermy::Font::Atlas* atlas = nullptr;
		ermy::Font::ASyncAtlasResult* aresult = nullptr;
		ermy::rendering::TextureID previewAtlasTex;
		ImTextureID previewAtlasTexImgui = 0;

		int glyphSize = 16;
		char name[64] = { 0 };
	};

	struct FontVertexCache
	{
		ermy::Font::VertexCache* vertexCache = nullptr;
		char name[64] = { 0 };
	};

	AssetDataType GetDataType() override { return AssetDataType::Font; }

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
	std::vector<ermy::u8> fontFileData;

	std::vector<FontAtlas> atlases;
	std::vector<FontVertexCache> vertexCaches;
};