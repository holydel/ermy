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

	AssetFolder* rootAssets = nullptr;
	AssetFolder* rootShaders = nullptr;

	AssetFolder* RescanAssets(const std::filesystem::path& pathTo);
public:
	ErmyProject();
	~ErmyProject();

	void MountToLocalDirectory(const std::string& filePath);

	static ErmyProject& Instance();

	bool RecompileAllInternalShaders();

	void Save();

	void DrawProjectSettings();

	AssetFolder* GetRootAssets() {
		return rootAssets;
	}

	AssetFolder* GetRootShaders() {
		return rootShaders;
	}
};