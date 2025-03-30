#include "project/eproject.h"

#include <iostream>
#include <fstream>
#include "shader_compiler/shader_compiler.h"
#include <imgui.h>
#include <ermy_pak.h>

#include "assets/texture_asset.h"
#include "assets/sound_asset.h"
#include <ermy_os_utils.h>
#include <ermy_mapped_writer.h>

#include "editor_file_utils.h"
float gCurrentTextureProgress = 0.0f;

namespace fs = std::filesystem;
using namespace ermy;

ErmyProject gCurrentProject;

const char* ProjectShadersBackendNames[] = {
    "SPIRV",
    "DXIL", 
    "MSL",
    "WGSL"
};

const char* PlatformBaseNames[] = {
    "Windows",
    "Android", 
    "Linux",
    "MacOS",
    "Emscripten",
    "iOS",
    "VisionOS"
};

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

    rootProjectCachePath = rootPath / fs::path("cache");

    if (!LoadAssetsCache())
    {
		//if (fs::exists(rootProjectCachePath))
		//{
		//	fs::remove_all(rootProjectCachePath);
		//}
        fs::create_directories(rootProjectCachePath);

        rootAssets = RescanAssets(rootAssetsPath);
    }
 
    UpdateWindowTitle();
}

void ErmyProject::UpdateWindowTitle()
{
    std::string title = projName;
    title += " (";
    title += platformInfos[currentPlatformIndex].platformName;
    title += ")";
    os_utils::SetNativeWindowTitle(title.c_str());
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

        if(auto att = project.attribute("current_platform"))
			currentPlatformIndex = att.as_int();

		if (auto att = project.attribute("AssetLastID"))
        {
            assetsID.SetCurrent(att.as_ullong());
        }

        auto platformNode = project.child("platform");
        while (platformNode)
        {
            PlatformInfo platform;  
            platform.Load(platformNode);
            platformInfos.push_back(platform);
            platformNode = platformNode.next_sibling("platform");
        }   

        //if no platfroms - create default one
        if(platformInfos.size() == 0)
        {
            auto platform = PlatformInfo();
            memcpy(platform.platformName,"Default",8);
            memcpy(platform.pakPath ,"assets/pak",10);
            platform.shadersBackend = ProjectShadersBackend::SPIRV;
            platform.platformBase = PlatformBase::Windows;
            platformInfos.push_back(platform);
            currentPlatformIndex = 0;
        }
    }
}

void ErmyProject::Save()
{
    xdoc.reset();
    auto project = xdoc.append_child("project");
    project.append_attribute("name").set_value(projName);
    project.append_attribute("current_platform").set_value(currentPlatformIndex);

	project.append_attribute("AssetLastID").set_value(assetsID.GetCurrent());

    for (auto& platform : platformInfos)
    {
        auto platformNode = project.append_child("platform");
        
        platform.Save(platformNode);   
    }

    xdoc.save_file(projPath.c_str());
    SaveAssetsCache();
}

void PlatformInfo::Save(pugi::xml_node& node)
{
    node.append_attribute("name").set_value(platformName);
    node.append_attribute("pak_path").set_value(pakPath);
    node.append_attribute("post_build_cmd").set_value(postBuildCmd);
    node.append_attribute("shaders_backend").set_value((int)shadersBackend);
    node.append_attribute("platform_base").set_value((int)platformBase);

    auto compressionNode = node.append_child("compression");
    for (int i = 0; i < (int)TextureAsset::TexturePurpose::TP_SOURCE; i++) {
        auto purposeNode = compressionNode.append_child("purpose");
        purposeNode.append_attribute("id").set_value(i);
        purposeNode.append_attribute("compression").set_value((int)compressionForPurpose[i]);
    }

    auto layoutsNode = node.append_child("vertex_layouts");
    for (auto& layout : vertexLayouts) {
        auto layoutNode = layoutsNode.append_child("layout");
        layout.Save(layoutNode);
    }
}

void PlatformInfo::Load(pugi::xml_node& node)
{
    if (auto att = node.attribute("name"))
        strcpy_s(platformName, att.value());
    if (auto att = node.attribute("pak_path"))
        strcpy_s(pakPath, att.value());
    if (auto att = node.attribute("post_build_cmd"))
        strcpy_s(postBuildCmd, att.value());
    if (auto att = node.attribute("shaders_backend"))
        shadersBackend = (ProjectShadersBackend)att.as_int();
    if (auto att = node.attribute("platform_base"))
        platformBase = (PlatformBase)att.as_int();

    auto compressionNode = node.child("compression");
    if (compressionNode) {
        for (auto purposeNode : compressionNode.children("purpose")) {
            int id = purposeNode.attribute("id").as_int();
            if (id <= (int)TextureAsset::TexturePurpose::TP_SOURCE) {
                compressionForPurpose[id] = (TextureAsset::TextureCompression)purposeNode.attribute("compression").as_int();
            }
        }
    }

    auto layoutsNode = node.child("vertex_layouts");
    if (layoutsNode) {
        vertexLayouts.clear();
        for (auto layoutNode : layoutsNode.children("layout")) {
            VertexLayoutDescription layout;
            layout.Load(layoutNode);
            vertexLayouts.push_back(layout);
        }
    }
}

void VertexLayoutDescription::Save(pugi::xml_node& node)
{
    node.append_attribute("name").set_value(vertexLayoutName);
    
    auto posNode = node.append_child("positions");
    for (auto format : positionsStream) {
        auto formatNode = posNode.append_child("format");
        formatNode.append_attribute("type").set_value((int)format);
    }

    auto fragNode = node.append_child("fragments");
    for (auto format : fragmentStream) {
        auto formatNode = fragNode.append_child("format");
        formatNode.append_attribute("type").set_value((int)format);
    }
}

void VertexLayoutDescription::Load(pugi::xml_node& node)
{
    if (auto att = node.attribute("name"))
        strcpy_s(vertexLayoutName, att.value());

    positionsStream.clear();
    fragmentStream.clear();

    auto posNode = node.child("positions");
    if (posNode) {
        for (auto formatNode : posNode.children("format")) {
            positionsStream.push_back((ermy::rendering::VertexFormat)formatNode.attribute("type").as_int());
        }
    }

    auto fragNode = node.child("fragments");
    if (fragNode) {
        for (auto formatNode : fragNode.children("format")) {
            fragmentStream.push_back((ermy::rendering::VertexFormat)formatNode.attribute("type").as_int());
        }
    }
}


void ErmyProject::DrawProjectSettings()
{
    if(isPakBuildingInProgress)
    {
        static const char* buildingStateNames[] = {
            "Idle",
            "Preprocessing",
            "Textures",
            "Sounds",
            "Geomertries",
            "Postprocessing"
        };
        
        ImGui::Begin("Building PAK", nullptr);
        ImGui::Text("Building PAK...");
        ImGui::Text("%s", buildingStateNames[(int)progressBuildingState.state]);

        if(progressBuildingState.state == ProgressBuildingState::State::Textures)
        {
            ImGui::Text("Textures: %d/%d", progressBuildingState.currentTexture, progressBuildingState.totalTextures);
            ImGui::ProgressBar((float)progressBuildingState.currentTexture / (float)progressBuildingState.totalTextures, ImVec2(0, 0));

            ImGui::ProgressBar(gCurrentTextureProgress, ImVec2(0, 0));
        }

        if(progressBuildingState.state == ProgressBuildingState::State::Sounds)
        {
            ImGui::Text("Sounds: %d/%d", progressBuildingState.currentSound, progressBuildingState.totalSounds);
            ImGui::ProgressBar((float)progressBuildingState.currentSound / (float)progressBuildingState.totalSounds, ImVec2(0, 0));
        }
        
        if(progressBuildingState.state == ProgressBuildingState::State::Geomertries)
        {
            ImGui::Text("Geomertries: %d/%d", progressBuildingState.currentGeometry, progressBuildingState.totalGeometries);
            ImGui::ProgressBar((float)progressBuildingState.currentGeometry / (float)progressBuildingState.totalGeometries, ImVec2(0, 0));
        }
        
        
        
        ImGui::End();
    }
    
    if (showSettings)
    {
        if (ImGui::Begin("Ermy Project Settings", &showSettings))
        {
            ImGui::InputText("Project Name:", projName, sizeof(projName), ImGuiInputTextFlags_NoHorizontalScroll);
  
            if (ImGui::BeginTable("Platforms", 2))
            {
                ImGui::TableSetupColumn("Platforms List", ImGuiTableColumnFlags_WidthFixed, 200.0f);
                ImGui::TableSetupColumn("Platform Settings", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                static int selectedPlatform = 0;
                if (platformInfos.size() == 0)
                {
                    selectedPlatform = -1;
                }
                else
                {
                    selectedPlatform = std::min(selectedPlatform, (int)platformInfos.size() - 1);
                }

                if (ImGui::Button("Add Platform"))
                {
                    platformInfos.push_back(PlatformInfo());
                    selectedPlatform = platformInfos.size() - 1;
                }

                for (int i = 0; i < platformInfos.size(); i++)
                {
                    if (ImGui::Selectable(platformInfos[i].platformName, selectedPlatform == i))
                    {
                        selectedPlatform = i;
                    }
                }

                ImGui::TableNextColumn();

                if (selectedPlatform >= 0)
                {
                    auto& platform = platformInfos[selectedPlatform];

                    ImGui::InputText("Platform Name", platform.platformName, sizeof(platform.platformName));

                    ImGui::InputText("PAK path:", platform.pakPath, sizeof(platform.pakPath), ImGuiInputTextFlags_NoHorizontalScroll);
                    ImGui::InputText("Post Build command:", platform.postBuildCmd, sizeof(platform.postBuildCmd), ImGuiInputTextFlags_NoHorizontalScroll);

                    int curBackend = (int)platform.shadersBackend;
                    if (ImGui::Combo("Shaders Backend", &curBackend, ProjectShadersBackendNames, std::size(ProjectShadersBackendNames)))
                    {
                        platform.shadersBackend = (ProjectShadersBackend)curBackend;
                    }

                    int curBase = (int)platform.platformBase;
                    if (ImGui::Combo("Platform Base", &curBase, PlatformBaseNames, std::size(PlatformBaseNames)))
                    {
                        platform.platformBase = (PlatformBase)curBase;
                    }

                    if (ImGui::CollapsingHeader("Texture Compression Settings"))
                    {
                        for (int i = 0; i < (int)TextureAsset::TexturePurpose::TP_SOURCE; i++) //TP_SOURCE is always TC_NONE
                        {
                            ImGui::Text("%s", TexturePurposeNames[i]);
                            ImGui::SameLine();
                            int curCompression = (int)platform.compressionForPurpose[i];
                            char label[32];
                            sprintf_s(label, "##compression%d", i);
                            if (ImGui::Combo(label, &curCompression, TextureCompressionNames, std::size(TextureCompressionNames)))
                            {
                                platform.compressionForPurpose[i] = (TextureAsset::TextureCompression)curCompression;
                            }
                        }
                    }

                    if (ImGui::CollapsingHeader("Vertex Layouts"))
                    {
                        if (ImGui::BeginTable("VertexLayouts", 2))
                        {
                            ImGui::TableSetupColumn("Layouts List", ImGuiTableColumnFlags_WidthFixed, 200.0f);
                            ImGui::TableSetupColumn("Layout Settings", ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableHeadersRow();

                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();

                            if (ImGui::Button("Add Vertex Layout"))
                            {
                                platform.vertexLayouts.push_back(VertexLayoutDescription());
                            }

                            static int selectedLayout = -1;
                            if (platform.vertexLayouts.size() == 0)
                            {
                                selectedLayout = -1;
                            }
                            else
                            {
                                selectedLayout = std::min(selectedLayout, (int)platform.vertexLayouts.size() - 1);
                            }

                            for (int i = 0; i < platform.vertexLayouts.size(); i++)
                            {
                                char label[32];
                                sprintf_s(label, "Layout %d", i);
                                if (ImGui::Selectable(platform.vertexLayouts[i].vertexLayoutName, selectedLayout == i))
                                {
                                    selectedLayout = i;
                                }
                            }

                            ImGui::TableNextColumn();

                            if (selectedLayout >= 0)
                            {
                                auto& layout = platform.vertexLayouts[selectedLayout];
                                ImGui::InputText("Layout Name", layout.vertexLayoutName, sizeof(layout.vertexLayoutName));

                                if (ImGui::CollapsingHeader("Position Stream"))
                                {
                                    if (ImGui::Button("Add Position Format"))
                                        layout.positionsStream.push_back(ermy::rendering::VertexFormat::UNKNOWN);

                                    for (int i = 0; i < layout.positionsStream.size(); i++)
                                    {
                                        char label[32];
                                        sprintf_s(label, "Position %d##pos%d", i, i);
                                        int format = (int)layout.positionsStream[i];
                                        if (ImGui::Combo(label, &format, ermy::rendering::VertexFormatNames, std::size(ermy::rendering::VertexFormatNames)))
                                        {
                                            layout.positionsStream[i] = (ermy::rendering::VertexFormat)format;
                                        }
                                        ImGui::SameLine();
                                        sprintf_s(label, "X##pos%d", i);
                                        if (ImGui::Button(label))
                                        {
                                            layout.positionsStream.erase(layout.positionsStream.begin() + i);
                                            i--;
                                        }
                                    }
                                }

                                if (ImGui::CollapsingHeader("Fragment Stream"))
                                {
                                    if (ImGui::Button("Add Fragment Format"))
                                        layout.fragmentStream.push_back(ermy::rendering::VertexFormat::UNKNOWN);

                                    for (int i = 0; i < layout.fragmentStream.size(); i++)
                                    {
                                        char label[32];
                                        sprintf_s(label, "Fragment %d##frag%d", i, i);
                                        int format = (int)layout.fragmentStream[i];
                                        if (ImGui::Combo(label, &format, ermy::rendering::VertexFormatNames, std::size(ermy::rendering::VertexFormatNames)))
                                        {
                                            layout.fragmentStream[i] = (ermy::rendering::VertexFormat)format;
                                        }
                                        ImGui::SameLine();
                                        sprintf_s(label, "X##frag%d", i);
                                        if (ImGui::Button(label))
                                        {
                                            layout.fragmentStream.erase(layout.fragmentStream.begin() + i);
                                            i--;
                                        }
                                    }
                                }

                                if (ImGui::Button("Delete Layout"))
                                {
                                    platform.vertexLayouts.erase(platform.vertexLayouts.begin() + selectedLayout);
                                    selectedLayout = -1;
                                }
                            }
                            ImGui::EndTable();
                        }
                    }

                    if (ImGui::Button("Delete Platform"))
                    {
                        platformInfos.erase(platformInfos.begin() + selectedPlatform);
                        selectedPlatform = -1;
                    }
                }
            }
            ImGui::EndTable();
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

bool ErmyProject::RebuildPAK(int platformIndex)
{
    if (!isPakBuildingInProgress)
    {
        isPakBuildingInProgress = true;

        buildingPakThread = new std::thread([this, platformIndex]() {
            RebuildPAKImmediate(platformIndex);
        });
    }
    return true;
}

bool ErmyProject::RebuildPAKImmediate(int platformIndex)
{
    progressBuildingState.state = ProgressBuildingState::State::Preprocessing;

    auto& currentPlatform = platformInfos[platformIndex];
    std::string pakPath = std::string(currentPlatform.pakPath);
    std::ofstream pak(pakPath, std::ios::binary);
    if (!pak.is_open())
    {
        return false;
    }

    writeBinary(pak, ermy::pak::PAK_MAGIC);
    //pak.write((const char*)&ermy::pak::PAK_MAGIC,sizeof(ermy::pak::PAK_MAGIC));
	
    AssetsToPak collectedAssets;
    //Collect assets to pak
	CollectAssets(rootAssets, collectedAssets);
    
    progressBuildingState.totalTextures = collectedAssets.textures.size();
    progressBuildingState.totalSounds = collectedAssets.sounds.size();

    progressBuildingState.state = ProgressBuildingState::State::Textures;


    std::vector<pak::TextureRawInfo> texMeta(collectedAssets.textures.size());
	for (int i = 0; i < collectedAssets.textures.size(); ++i)
	{
        progressBuildingState.currentTexture = i;
		auto tex = collectedAssets.textures[i];
		auto texData = tex->GetData();
		TextureAsset* texAsset = static_cast<TextureAsset*>(texData); //guaranteed to be texture asset
        texAsset->ProcessAssetToPak();

		texMeta[i].width = texAsset->width;
		texMeta[i].height = texAsset->height;
		texMeta[i].depth = texAsset->depth;
		texMeta[i].numMips = texAsset->numMips;
		texMeta[i].numLayers = texAsset->numLayers;
		texMeta[i].isCubemap = texAsset->isCubemap;
		texMeta[i].texelSourceFormat = texAsset->texelTargetFormat;
		texMeta[i].dataSize = texAsset->GetPakData().size_bytes(); //compress mips if target format not euqal source fromat
	}
    progressBuildingState.state = ProgressBuildingState::State::Sounds;
   
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

    writeVector(pak, texMeta);
    writeVector(pak, soundMeta);
    progressBuildingState.state = ProgressBuildingState::State::Postprocessing;
	for (int i = 0; i < collectedAssets.textures.size(); ++i)
	{
       
		auto tex = collectedAssets.textures[i];
		auto texData = tex->GetData();
		auto texAsset = static_cast<TextureAsset*>(texData); //guaranteed to be texture asset
		pak.write((const char*)texAsset->GetPakData().data(), texAsset->GetPakData().size_bytes());
	}
	

	for (int i = 0; i < collectedAssets.sounds.size(); ++i)
	{
        progressBuildingState.currentSound = i;
		auto sound = collectedAssets.sounds[i];
		auto soundData = sound->GetData();
		auto soundAsset = static_cast<SoundAsset*>(soundData); //guaranteed to be sound asset
		copyFile(sound->GetSourcePath(), pak);
	}
	

    isPakBuildingInProgress = false;
	return true;
}