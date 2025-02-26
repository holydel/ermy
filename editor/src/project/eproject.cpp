#include "project/eproject.h"

#include <iostream>
#include <fstream>
#include "shader_compiler/shader_compiler.h"
#include <imgui.h>
#include <ermy_pak.h>

#include "assets/texture_asset.h"
#include "assets/sound_asset.h"

namespace fs = std::filesystem;
using namespace ermy;

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
	assetsCachePath = rootPath / fs::path("assets_cache.xml");

    Load();

    rootAssetsPath = rootPath / fs::path("assets");

    if (!LoadAssetsCache())
    {
        rootAssets = RescanAssets(rootAssetsPath);
    }
 
    int a = 42;
}

ErmyProject& ErmyProject::Instance()
{
	return gCurrentProject;
}

bool ErmyProject::RecompileAllShaders()
{
    return true;
}

bool ErmyProject::RecompileAllEditorShaders()
{
    ShaderCompiler::Instance().CompileAllEditorShaders();
    return true;
}

bool ErmyProject::RecompileAllInternalShaders()
{
    ShaderCompiler::Instance().CompileAllEngineShaders();

    //for (auto& s : shaders)
    //{
    //    ShaderCompiler::Instance().CompileAllInternalShaders(s.filePath);        
    //}
    return true;
}

bool ErmyProject::LoadAssetsCache()
{
    xdoc_assets.load_file(assetsCachePath.c_str());
    auto assets = xdoc_assets.child("assets");
    if (assets)
    {
        auto assetRootNode = assets.child("Asset");
		if (!assetRootNode)
			return false;

		rootAssets = new AssetFolder();
		rootAssets->Load(assetRootNode, rootAssetsPath);

		return true;
    }

    return false;
}


void ErmyProject::SaveAssetsCache()
{
    xdoc_assets.reset();
    auto assets = xdoc_assets.append_child("assets");

    rootAssets->Save(assets, rootAssetsPath);
    xdoc_assets.save_file(assetsCachePath.c_str());
}

void ErmyProject::Load()
{
    xdoc.load_file(projPath.c_str());
    auto project = xdoc.child("project");
    if (project)
    {
        if(auto att = project.attribute("name"))        
            strcpy_s(projName, att.value());
        if (auto att = project.attribute("pakPath"))
            strcpy_s(pakPath, att.value());
        if (auto att = project.attribute("postBuildCmd"))
            strcpy_s(postBuildCmd, att.value());
    }
}

void ErmyProject::Save()
{
    xdoc.reset();
    auto project = xdoc.append_child("project");
    project.append_attribute("name").set_value(projName);

    project.append_attribute("pakPath").set_value(pakPath);
    project.append_attribute("postBuildCmd").set_value(postBuildCmd);

    xdoc.save_file(projPath.c_str());

    SaveAssetsCache();
}

void ErmyProject::DrawProjectSettings()
{
    if (showSettings)
    {
        if (ImGui::Begin("Ermy Project Settings", &showSettings))
        {
            ImGui::InputText("Project Name:", projName, sizeof(projName), ImGuiInputTextFlags_NoHorizontalScroll);

            ImGui::InputText("PAK path:", pakPath, sizeof(pakPath), ImGuiInputTextFlags_NoHorizontalScroll);
            ImGui::InputText("Post Build command:", postBuildCmd, sizeof(postBuildCmd), ImGuiInputTextFlags_NoHorizontalScroll);
            
        }
        ImGui::End();
    }

}

struct AssetsToPak
{
    std::vector<Asset*> textures;
    std::vector<Asset*> sounds;

    AssetsToPak()
    {
        textures.reserve(1024);
        sounds.reserve(256);
    }
};

void CollectAssets(AssetFolder* folder, AssetsToPak& out)
{
    for (auto sub : folder->subdirectories)
    {
		CollectAssets(sub, out);
    }

	for (auto asset : folder->content)
	{
        if (asset->NeedToIncludeInPAK())
        {
            if (!asset->GetData())
            {
				asset->Import();
            }

            if (asset->GetData())
            {
                if (asset->GetData()->GetDataType() == AssetDataType::Texture)
                {
                    out.textures.push_back(asset);
                }

                if (asset->GetData()->GetDataType() == AssetDataType::Sound)
                {
                    out.sounds.push_back(asset);
                }
            }            
        }
	}
}

bool copyFile(const std::filesystem::path& sourcePath, std::ofstream& destinationStream) {
    std::ifstream sourceFile(sourcePath, std::ios::binary);
    if (!sourceFile) {
        return false;
    }


    if (!destinationStream) {
        sourceFile.close();
        return false;
    }


    char buffer[4096]; 
    while (sourceFile.read(buffer, sizeof(buffer))) {
        destinationStream.write(buffer, sourceFile.gcount());
    }

    if (sourceFile.eof() && sourceFile.gcount() > 0) {
        destinationStream.write(buffer, sourceFile.gcount());
    }

    sourceFile.close();

    return true;
}


bool ErmyProject::RebuildPAK()
{
    std::ofstream pak(pakPath, std::ios::binary);
    if (!pak.is_open())
    {
        return false;
    }

    pak << ermy::pak::PAK_MAGIC;
	
    AssetsToPak collectedAssets;
    //Collect assets to pak
	CollectAssets(rootAssets, collectedAssets);
    
    std::vector<pak::TextureRawInfo> texMeta(collectedAssets.textures.size());
	for (int i = 0; i < collectedAssets.textures.size(); ++i)
	{
		auto tex = collectedAssets.textures[i];
		auto texData = tex->GetData();
		auto texAsset = static_cast<TextureAsset*>(texData); //guaranteed to be texture asset
		texMeta[i].width = texAsset->width;
		texMeta[i].height = texAsset->height;
		texMeta[i].depth = texAsset->depth;
		texMeta[i].numMips = texAsset->numMips;
		texMeta[i].numLayers = texAsset->numLayers;
		texMeta[i].isCubemap = texAsset->isCubemap;
		texMeta[i].texelFormat = texAsset->texelFormat;
		texMeta[i].dataSize = texAsset->dataSize;
	}

	std::vector<pak::SoundRawInfo> soundMeta(collectedAssets.sounds.size());
	for (int i = 0; i < collectedAssets.sounds.size(); ++i)
	{
		auto sound = collectedAssets.sounds[i];
		auto soundData = sound->GetData();
		auto soundAsset = static_cast<SoundAsset*>(soundData); //guaranteed to be sound asset
		soundMeta[i].numChannels = soundAsset->channels;
        soundMeta[i].sampleRate = 44100;
        soundMeta[i].dataSize = sound->FileSize();
	}

	pak << texMeta.size();
	pak.write((const char*)texMeta.data(), texMeta.size() * sizeof(pak::TextureRawInfo));

	pak << soundMeta.size();
	pak.write((const char*)soundMeta.data(), soundMeta.size() * sizeof(pak::SoundRawInfo));

	for (int i = 0; i < collectedAssets.textures.size(); ++i)
	{
		auto tex = collectedAssets.textures[i];
		auto texData = tex->GetData();
		auto texAsset = static_cast<TextureAsset*>(texData); //guaranteed to be texture asset
		pak.write((const char*)texAsset->data, texAsset->dataSize);
	}
	
	for (int i = 0; i < collectedAssets.sounds.size(); ++i)
	{
		auto sound = collectedAssets.sounds[i];
		auto soundData = sound->GetData();
		auto soundAsset = static_cast<SoundAsset*>(soundData); //guaranteed to be sound asset
		copyFile(sound->GetSourcePath(), pak);
	}
	
	return true;
}