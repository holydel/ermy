#include "project/eproject.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include "shader_compiler/shader_compiler.h"
namespace fs = std::filesystem;

ErmyProject gCurrentProject;


ErmyProject::ErmyProject()
{

}

ErmyProject::~ErmyProject()
{

}

void ReadAllFilesFromDirectory(const fs::path& path, std::vector<fs::path>& outFiles)
{
    outFiles.clear();

    if (fs::exists(path) && fs::is_directory(path)) {
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (fs::is_regular_file(entry.status())) {
                outFiles.push_back(entry.path());
            }
        }
    }
}

std::string ReadAllFile(const fs::path& path)
{
    std::ifstream fileStream(path, std::ios::in | std::ios::binary);
    // Seek to the end of the file to get its size
    fileStream.seekg(0, std::ios::end);
    std::size_t fileSize = fileStream.tellg();
    fileStream.seekg(0, std::ios::beg);

    // Read the file contents into a string
    std::string fileContent(fileSize, '\0');
    fileStream.read(&fileContent[0], fileSize);

    return fileContent;
}

void ErmyProject::MountToLocalDirectory(const std::string& filePath)
{
    std::vector<fs::path> allShaders;
    allShaders.reserve(64);

    auto shadersPath = fs::path(filePath) / fs::path("shaders");

    ReadAllFilesFromDirectory(shadersPath, allShaders);
    shaders.reserve(allShaders.size());
    for (auto& s : allShaders)
    {
        shaders.push_back({ s,ReadAllFile(s) });
    }
    int a = 42;
}

ErmyProject& ErmyProject::Instance()
{
	return gCurrentProject;
}

bool ErmyProject::RecompileAllShaders()
{
    ShaderCompiler::Instance().CompileAllInternalShaders();

    //for (auto& s : shaders)
    //{
    //    ShaderCompiler::Instance().CompileAllInternalShaders(s.filePath);        
    //}
    return true;
}