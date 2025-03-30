#include <assets/asset.h>
#include <assets/assets_loader.h>

#include <assets/texture_asset.h>
#include <assets/video_asset.h>
#include <assets/sound_asset.h>
#include <assets/geometry_asset.h>

#include <imgui.h>
#include <inttypes.h>

#include <ermy_utils.h>
#include <chrono>

#include "project/eproject.h"

namespace fs = std::filesystem;
using namespace ermy;

void AssetFolder::Scan(const std::filesystem::path& path)
{
	source = path;
	name = path.filename().string();
	lastWriteTime = fs::last_write_time(path);

	CalculateAssetName();

	for (const auto& entry : fs::directory_iterator(path)) {
		if (fs::is_directory(entry.status()))
		{
			AssetFolder* folder = new AssetFolder();
			folder->Scan(entry.path());

			folderData.numFiles += folder->folderData.numFiles;
			folderData.totalSize += folder->folderData.totalSize;

			content.push_back(folder);
			subdirectories.push_back(folder);
		}
		else
		{
			Asset* asset = Asset::CreateFromPath(entry.path());
			content.push_back(asset);

			++folderData.numFiles;
			folderData.totalSize += asset->FileSize();
		}
	}

	ID = ErmyProject::Instance().GetNextAssetID();
}

pugi::xml_node AssetFolder::Save(pugi::xml_node& node, const std::filesystem::path& rootPath)
{
	pugi::xml_node newNode = Asset::Save(node, rootPath);
	newNode.append_attribute("files").set_value(folderData.numFiles);
	newNode.append_attribute("totalSize").set_value(folderData.totalSize);
	auto folderNode = newNode.append_child("Folder");
	for (auto asset : content)
	{
		asset->Save(folderNode, source);
	}
	return newNode;
}

void AssetFolder::Load(pugi::xml_node& node, const std::filesystem::path& currentPath)
{
	Asset::Load(node, currentPath);
	folderData.numFiles = node.attribute("files").as_uint();
	folderData.totalSize = node.attribute("totalSize").as_ullong();
	auto folderNode = node.child("Folder");
	for (auto assetNode : folderNode.children())
	{
		auto atype = static_cast<AssetType>(assetNode.attribute("type").as_int());
		if (atype == AssetType::Folder)
		{
			AssetFolder* folder = new AssetFolder();			
			folder->Load(assetNode, source);
			content.push_back(folder);
			subdirectories.push_back(folder);
		}
		else
		{
			Asset* asset = new Asset();
			asset->Load(assetNode, source);
			content.push_back(asset);
		}
	}
}

Asset* Asset::CreateFromPath(const std::filesystem::path path)
{
	Asset* result = new Asset();
	result->source = path;
	result->name = path.filename().string();
	result->fileSize = fs::file_size(path);
	result->lastWriteTime = fs::last_write_time(path);
	result->CalculateAssetName();
	result->ID = ErmyProject::Instance().GetNextAssetID();
	return result;
}

void Asset::CalculateAssetName()
{
	//calculate asset name to fit into 128x128 preview
	int chN = 18;// only 18 chars fits to row
	assetNameRows = static_cast<u8>(name.size() / (chN + 1));

	if (assetNameRows > 0)
	{
		assetName = name.substr(0, chN);
		for (int i = 0; i < assetNameRows; ++i)
		{
			assetName += "\n";
			assetName += name.substr((i + 1) * chN, chN);
		}
	}
	else
	{
		assetName = name;
	}
}

void Asset::RegenerateStaticPreviewTexture()
{
	if (!needRegenerateStaticPreview)
		return;

	needRegenerateStaticPreview = false;

	if (assetPreviewTexStatic == 0)
	{

		fs::path cachedPreviewPath = ErmyProject::Instance().GetProjectCachePath() / (std::to_string(ID) + ".preview.bc1");
		std::vector<u8> bc1_data;

		if (fs::exists(cachedPreviewPath))
		{
			bc1_data = ermy_utils::file::ReadFile(cachedPreviewPath);
		}
		else
		{
			if (data == nullptr)
			{
				Import();
			}

			if (data != nullptr)
			{
				bc1_data = GetData()->GetStaticPreviewData();

				if (!bc1_data.empty())
					ermy_utils::file::WriteFile(cachedPreviewPath, bc1_data);
			}
		}

		if (bc1_data.empty())
		{
			return;
		}

		ermy::rendering::TextureDesc descBC1;
		descBC1.width = 128;
		descBC1.height = 128;
		descBC1.depth = 1;
		descBC1.isCubemap = false;
		descBC1.numLayers = 1;
		descBC1.numMips = 1;
		descBC1.isSparse = false;
		descBC1.texelSourceFormat = ermy::rendering::Format::BC1;
		descBC1.dataSize = (int)bc1_data.size();
		descBC1.pixelsData = bc1_data.data();
		descBC1.debugName = "StaticPreviewBC1";

		previewTextureStatic = ermy::rendering::CreateDedicatedTexture(descBC1);
		assetPreviewTexStatic = ermy::rendering::GetTextureDescriptor(previewTextureStatic);
	}
}

void Asset::DrawProps()
{
	auto strPath = source.string();
	ImGui::InputText("P", (char*)strPath.c_str(), strPath.size(), ImGuiInputTextFlags_ReadOnly);
	ImGui::Text("Asset file size: %s", ermy_utils::string::humanReadableFileSize(fileSize).c_str());
	ImGui::Text("Modified: %s", ermy_utils::string::humanReadableFileDate(lastWriteTime).c_str());
	ImGui::Checkbox("Include in PAK", &includeInPAK);
	if (data)
		data->DrawPreview();
}

pugi::xml_node Asset::Save(pugi::xml_node& node, const std::filesystem::path& rootPath)
{
	auto assetNode = node.append_child("Asset");
	assetNode.append_attribute("type").set_value(static_cast<int>(GetType()));

	fs::path additionalPath = source.lexically_relative(rootPath);

	assetNode.append_attribute("path").set_value(additionalPath.string().c_str());
	assetNode.append_attribute("pak").set_value(includeInPAK);
	assetNode.append_attribute("date").set_value(lastWriteTime.time_since_epoch().count());
	assetNode.append_attribute("size").set_value(fileSize);
	assetNode.append_attribute("name").set_value(name.c_str());
	assetNode.append_attribute("ID").set_value(ID);
	if (GetData())
	{
		auto dataNode = assetNode.append_child("Data");
		GetData()->Save(dataNode);
	}
	return assetNode;
}

void AssetData::Save(pugi::xml_node& node)
{
	node.append_attribute("type").set_value(static_cast<int>(GetDataType()));
}

fs::file_time_type u64_to_file_time_type(uint64_t ticks_since_epoch)
{
	std::filesystem::file_time_type::duration d(ticks_since_epoch);
	fs::file_time_type lastWriteTime{ d };
	return lastWriteTime;
}

void Asset::Load(pugi::xml_node& node, const std::filesystem::path& currentPath)
{
	std::filesystem::path localPath = node.attribute("path").as_string();	
	source = currentPath / localPath;
	includeInPAK = node.attribute("pak").as_bool();
	lastWriteTime = u64_to_file_time_type(node.attribute("date").as_ullong());
	fileSize = node.attribute("size").as_ullong();
	name = node.attribute("name").as_string();
	ID = node.attribute("ID").as_ullong();
	CalculateAssetName();

	auto dataNode = node.child("Data");
	if (dataNode)
	{
		auto dataType = static_cast<AssetDataType>(dataNode.attribute("type").as_int());
		
		if(dataType != AssetDataType::UNKNOWN)
		{
			Import();

			//data = AssetData::CreateFromType(dataType);
			if(data)
			{
				data->Load(dataNode);
				
			}
		}
		
	}
}

void BinaryAssetData::DrawPreview()
{
	ImGui::Text("Blob size: %" PRIu64, blobSize);
}

void AssetFolder::FolderData::DrawPreview()
{
	ImGui::Text("Folder size: %s", ermy_utils::string::humanReadableFileSize(totalSize).c_str());
	ImGui::Text("Folder files: %" PRIu32, numFiles);
}

AssetData* AssetData::CreateFromType(AssetDataType type)
{
	switch (type)
	{
		case AssetDataType::Texture:
			return new TextureAsset();
		case AssetDataType::Video:
			return new VideoTextureAsset();
		case AssetDataType::Sound:
			return new SoundAsset();
		case AssetDataType::Geometry:
			return new GeometryAsset();
	}

	return nullptr;
}	