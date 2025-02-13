#include "assets/assets_loader.h"
#include "assets/FFMPEGLoader.h"
#include "assets/OpenImageLoader.h"
#include "assets/KTXLoader.h"
#include "assets/AssimpLoader.h"
#include "assets/texture_asset.h"
#include "assets/video_asset.h"
#include "assets/geometry_asset.h"
#include "assets/sound_asset.h"

#include <vector>
#include <tuple>
#include <unordered_map>

using namespace editor::asset::loader;

std::unordered_map<std::string, std::vector<AssetsLoader*>> gAllLoaders;

FFMpegLoader* gFfmpegLoader = nullptr;
OpenImageLoader* gOpenImageLoader = nullptr;
AssimpLoader* gAssimpLoader = nullptr;

void PopulateLoaderExtensions(AssetsLoader* loader)
{
    auto exts = loader->SupportedExtensions();
    for (const auto& e : exts)
    {
        gAllLoaders[e].push_back(loader);
    }
}

bool AssetsLoader::Initialize()
{
    gOpenImageLoader = new OpenImageLoader();
    gFfmpegLoader = new FFMpegLoader();
    gAssimpLoader = new AssimpLoader();

    //It's important to populate specific loaders first. The order matters.
    PopulateLoaderExtensions(gOpenImageLoader);
    PopulateLoaderExtensions(gAssimpLoader);
    PopulateLoaderExtensions(gFfmpegLoader);
    return true;
}

bool AssetsLoader::Shutdown()
{
    return true;
}


void Asset::Import()
{
    data = new TextureAsset();
}

