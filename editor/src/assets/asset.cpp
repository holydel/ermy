#include <assets/asset.h>
#include <assets/assets_loader.h>

#include <assets/texture_asset.h>
#include <assets/video_asset.h>
#include <assets/sound_asset.h>
#include <assets/geometry_asset.h>

#include <imgui.h>
#include <inttypes.h>

#include <ermy_utils.h>

namespace fs = std::filesystem;
using namespace ermy;

void AssetFolder::Scan(const std::filesystem::path& path)
{
	source = path;
	name = path.filename().string();
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
}

Asset* Asset::CreateFromPath(const std::filesystem::path path)
{
	Asset* result = new Asset();
	result->source = path;
	result->name = path.filename().string();
	result->fileSize = fs::file_size(path);

	result->CalculateAssetName();
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

void Asset::DrawProps()
{
	auto strPath = source.string();
	ImGui::InputText("P", (char*)strPath.c_str(), strPath.size(), ImGuiInputTextFlags_ReadOnly);
	ImGui::Text("Asset file size: %s", ermy_utils::string::humanReadableFileSize(fileSize).c_str());

	if (data)
		data->DrawPreview();
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
