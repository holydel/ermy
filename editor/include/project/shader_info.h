#pragma once
#include <filesystem>
#include <string>

struct ShaderInfo
{
	std::filesystem::path filePath;
	std::string fileContentCached;
};