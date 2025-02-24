#include "assets/assets_loader.h"
#include "assets/FFMPEGLoader.h"
#include "assets/OpenImageLoader.h"
#include "assets/KTXLoader.h"
#include "assets/DDSLoader.h"
#include "assets/AssimpLoader.h"
#include "assets/texture_asset.h"
#include "assets/video_asset.h"
#include "assets/geometry_asset.h"
#include "assets/sound_asset.h"
#include "assets/SoundsLoader.h"

#include <vector>
#include <tuple>
#include <unordered_map>
#include <ermy_utils.h>

using namespace editor::asset::loader;

std::unordered_map<std::string, std::vector<AssetsLoader*>> gAllLoaders;

FFMpegLoader* gFfmpegLoader = nullptr;
OpenImageLoader* gOpenImageLoader = nullptr;
AssimpLoader* gAssimpLoader = nullptr;
KTXLoader* gKTXLoader = nullptr;
DDSLoader* gDDSLoader = nullptr;
SoundsLoader* gSoundsLoader = nullptr;

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
    gKTXLoader = new KTXLoader();
    gDDSLoader = new DDSLoader();
	gSoundsLoader = new SoundsLoader();
    //It's important to populate specific loaders first. The order matters.
	PopulateLoaderExtensions(gSoundsLoader);
    PopulateLoaderExtensions(gKTXLoader);
    PopulateLoaderExtensions(gDDSLoader);
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
    if (source.has_extension())
    {
        std::string ext = ermy_utils::string::toLower(source.extension().string().substr(1));
        
        auto loaderIt = gAllLoaders.find(ext);

        if (loaderIt != gAllLoaders.end())
        {
            for (auto* loader : loaderIt->second)
            {
                data = loader->Load(source);
                if (data)
                    return;
            }
        }
    }

    data = new BinaryAssetData();
}

static ermy::u64 AssetID = 0;

Asset::Asset()
{
    ID = ++AssetID;
}

