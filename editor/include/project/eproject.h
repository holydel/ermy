#pragma once
#include <ermy_api.h>
#include <string>
#include <filesystem>
#include "shader_info.h"
#include <vector>
#include "pugixml.hpp"
#include "assets/asset.h"

class ErmyProject
{
	pugi::xml_document xdoc;
	bool wasModified = false;
	std::filesystem::path rootPath;
	std::filesystem::path projPath;

	char projName[64] = { 'U','n','n','a','m','e','d',' ','E','r','m','y',' ','P','r','o','j','e','c','t' };
	char pakPath[256] = { 0 };
	char postBuildCmd[256] = { 0 };
	AssetFolder* rootAssets = nullptr;
	AssetFolder* rootShaders = nullptr;

	AssetFolder* RescanAssets(const std::filesystem::path& pathTo);
	bool showSettings = false;
public:
	ErmyProject();
	~ErmyProject();

	void MountToLocalDirectory(const std::string& filePath);

	static ErmyProject& Instance();

	bool RecompileAllInternalShaders();
	bool RecompileAllShaders();
	bool RecompileAllEditorShaders();
	bool RebuildPAK();
	void Load();
	void Save();

	void DrawProjectSettings();

	void ShowSettings()
	{
		showSettings = true;
	}

	AssetFolder* GetRootAssets() {
		return rootAssets;
	}

	AssetFolder* GetRootShaders() {
		return rootShaders;
	}
};