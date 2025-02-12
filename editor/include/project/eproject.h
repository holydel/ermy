#pragma once
#include <ermy_api.h>
#include <string>
#include <filesystem>
#include "shader_info.h"
#include <vector>
#include "pugixml.hpp"

class ErmyProject
{
	std::vector<ShaderInfo> shaders;
	pugi::xml_document xdoc;
	bool wasModified = false;
	std::filesystem::path rootPath;
	std::filesystem::path projPath;

	char projName[64] = {'U','n','n','a','m','e','d',' ','E','r','m','y',' ','P','r','o','j','e','c','t'};
public:
	ErmyProject();
	~ErmyProject();

	void MountToLocalDirectory(const std::string& filePath);

	static ErmyProject& Instance();

	bool RecompileAllInternalShaders();

	void Save();

	void DrawProjectSettings();
};