#pragma once
#include <ermy_api.h>
#include <string>

#include "shader_info.h"
#include <vector>

class ErmyProject
{
	std::vector<ShaderInfo> shaders;
public:
	ErmyProject();
	~ErmyProject();

	void MountToLocalDirectory(const std::string& filePath);

	static ErmyProject& Instance();

	bool RecompileAllShaders();
};