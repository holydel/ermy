#pragma once
#include <ermy_api.h>
#include <string>
#include <filesystem>
#include "shader_info.h"
#include <vector>
#include "pugixml.hpp"
#include "assets/asset.h"
#include "assets/texture_asset.h"
enum class ProjectShadersBackend
{
	SPIRV,
	DXIL,
	MSL,
	WGSL	
};

extern const char* ProjectShadersBackendNames[];

enum class PlatformBase
{
	Windows,
	Android,
	Linux,
	MacOS,
	Emscripten,
	iOS,
	VisionOS
};

extern const char* PlatformBaseNames[];

struct VertexLayoutDescription
{
	char vertexLayoutName[64] = { 'U','n','n','a','m','e','d',' ','v','e','r','t','e','x',' ','l','a','y','o','u' ,'t' };
	std::vector<ermy::rendering::VertexFormat> positionsStream;
	std::vector<ermy::rendering::VertexFormat> fragmentStream;

	void Save(pugi::xml_node& node);
	void Load(pugi::xml_node& node);
};
struct PlatformInfo
{
	char platformName[64] = { 'U','n','n','a','m','e','d',' ','E','r','m','y',' ','P','l','a','t','f','o','r' ,'m' };

	char pakPath[256] = { 0 };
	char postBuildCmd[256] = { 0 };

	ProjectShadersBackend shadersBackend = ProjectShadersBackend::SPIRV;
	PlatformBase platformBase = PlatformBase::Windows;

	TextureAsset::TextureCompression compressionForPurpose[(int)TextureAsset::TexturePurpose::TP_SOURCE + 1] = {
		TextureAsset::TextureCompression::TC_BC1_SRGB,
		TextureAsset::TextureCompression::TC_BC1_UNORM,
		TextureAsset::TextureCompression::TC_BC5_UNORM,
		TextureAsset::TextureCompression::TC_BC6H_UNORM,
		TextureAsset::TextureCompression::TC_NONE
	};

	std::vector<VertexLayoutDescription> vertexLayouts;

	void Save(pugi::xml_node& node);
	void Load(pugi::xml_node& node);
};

class ErmyProject
{	
	pugi::xml_document xdoc;
	pugi::xml_document xdoc_assets;
	bool wasModified = false;
	std::filesystem::path rootPath;
	std::filesystem::path projPath;
	std::filesystem::path assetsCachePath;

	std::filesystem::path rootAssetsPath;

	char projName[64] = { 'U','n','n','a','m','e','d',' ','E','r','m','y',' ','P','r','o','j','e','c','t' };

	AssetFolder* rootAssets = nullptr;

	AssetFolder* RescanAssets(const std::filesystem::path& pathTo);
	bool showSettings = false;

	std::vector<PlatformInfo> platformInfos;
public:
	ErmyProject();
	~ErmyProject();

	void MountToLocalDirectory(const std::string& filePath);

	static ErmyProject& Instance();

	bool RecompileAllInternalShaders();
	bool RecompileAllShaders();
	bool RecompileAllEditorShaders();
	bool RebuildPAK(int platformIndex);
	void Load();
	void Save();
	bool LoadAssetsCache();
	void SaveAssetsCache();
	void DrawProjectSettings();

	void ShowSettings()
	{
		showSettings = true;
	}

	AssetFolder* GetRootAssets() {
		return rootAssets;
	}
};