#include "project/eproject.h"

#include <iostream>
#include <fstream>
#include "shader_compiler/shader_compiler.h"
#include <imgui.h>

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

AssetFolder* ErmyProject::RescanAssets(const std::filesystem::path& pathTo)
{
    AssetFolder* root = new AssetFolder();
    root->Scan(pathTo);
    return root;
}

void ErmyProject::MountToLocalDirectory(const std::string& filePath)
{
    rootPath = fs::path(filePath);
    projPath = rootPath / fs::path("project.eproj");

    xdoc.load_file(projPath.c_str());
    auto project = xdoc.child("project");
    if (project)
    {
        auto nameAtt = project.attribute("name");
        if (nameAtt)
        {
            strcpy_s(projName, nameAtt.value());
        }
    }

    rootShaders = RescanAssets(rootPath / fs::path("shaders"));
    rootAssets = RescanAssets(rootPath / fs::path("assets"));

    int a = 42;
}

ErmyProject& ErmyProject::Instance()
{
	return gCurrentProject;
}

bool ErmyProject::RecompileAllInternalShaders()
{
    ShaderCompiler::Instance().CompileAllInternalShaders();

    //for (auto& s : shaders)
    //{
    //    ShaderCompiler::Instance().CompileAllInternalShaders(s.filePath);        
    //}
    return true;
}

void ErmyProject::Save()
{
    xdoc.reset();
    auto project = xdoc.append_child("project");
    project.append_attribute("name").set_value(projName);

    xdoc.save_file(projPath.c_str());
}

void ErmyProject::DrawProjectSettings()
{
    if (showSettings)
    {
        if (ImGui::Begin("Ermy Project Settings", &showSettings))
        {
            ImGui::InputText("Project Name:", projName, sizeof(projName), ImGuiInputTextFlags_NoHorizontalScroll);
            
        }
        ImGui::End();
    }

}