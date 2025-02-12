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
	for (const auto& entry : fs::directory_iterator(path)) {
		if (fs::is_directory(entry.status()))
		{
			AssetFolder* folder = new AssetFolder();
			folder->Scan(entry.path());

			content.push_back(folder);
		}
		else
		{
			Asset* asset = Asset::CreateFromPath(entry.path());
			//asset->source = entry.path();

			content.push_back(asset);
		}
	}
}

Asset* Asset::CreateFromPath(const std::filesystem::path path)
{
	AssetType atype = AssetType::UNKNOWN;
	AssetLoaderType ltype = AssetLoaderType::UNKNOWN;

	editor::asset::loader::AssetsLoader::DescribeAssetLoaderByPath(path, atype, ltype);

	Asset* result = nullptr;

	switch (atype)
	{
	case AssetType::UNKNOWN: //fall		
	case AssetType::Folder: //fall		
	case AssetType::AUTO:
		return nullptr;
		break;
	case AssetType::Texture:
		result = new TextureAsset();
		break;
	case AssetType::Geometry:
		result = new GeometryAsset();
		break;
	case AssetType::Sound:
		result = new SoundAsset();
		break;
	case AssetType::VideoTexture:
		result = new VideoTextureAsset();
		break;
	default:
		break;
	}
	if(!result)
		return nullptr;

	result->importerToLoad = ltype;

	return result;
}
