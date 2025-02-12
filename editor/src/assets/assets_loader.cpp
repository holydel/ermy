#include "assets/assets_loader.h"
#include "assets/FFMPEGLoader.h"
#include "assets/OpenImageLoader.h"
#include "assets/KTXLoader.h"
#include "assets/AssimpLoader.h"

#include <vector>
#include <tuple>
#include <unordered_map>

using namespace editor::asset::loader;

std::unordered_map<std::string, std::vector<std::tuple<AssetType, AssetsLoader*>>> gAllLoaders;

bool AssetsLoader::Initialize()
{
    return true;
}

bool AssetsLoader::Shotdown()
{
    return true;
}

bool AssetsLoader::DescribeAssetLoaderByPath(const std::filesystem::path path, AssetType& atype, AssetLoaderType& ltype)
{
    atype = AssetType::Texture;
    ltype = AssetLoaderType::OpenImage;

    return true;
}
