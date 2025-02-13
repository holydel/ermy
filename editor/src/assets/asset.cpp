#include <assets/asset.h>
#include <assets/assets_loader.h>

#include <assets/texture_asset.h>
#include <assets/video_asset.h>
#include <assets/sound_asset.h>
#include <assets/geometry_asset.h>

namespace fs = std::filesystem;

void AssetFolder::Scan(const std::filesystem::path& path)
{
	source = path;
	name = path.filename().string();

	for (const auto& entry : fs::directory_iterator(path)) {
		if (fs::is_directory(entry.status()))
		{
			AssetFolder* folder = new AssetFolder();
			folder->Scan(entry.path());

			content.push_back(folder);
			subdirectories.push_back(folder);
		}
		else
		{
			Asset* asset = Asset::CreateFromPath(entry.path());
			content.push_back(asset);
		}
	}
}

Asset* Asset::CreateFromPath(const std::filesystem::path path)
{
	Asset* result = new Asset();
	result->source = path;
	result->name = path.filename().string();
	return result;
}
