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
#include "assets/CompressonatorLoader.h"

#include <vector>
#include <tuple>
#include <unordered_map>
#include <ermy_utils.h>
#include <project/eproject.h>
#include <fstream>
using namespace editor::asset::loader;

std::unordered_map<std::string, std::vector<AssetsLoader*>> gAllLoaders;

FFMpegLoader* gFfmpegLoader = nullptr;
OpenImageLoader* gOpenImageLoader = nullptr;
AssimpLoader* gAssimpLoader = nullptr;
KTXLoader* gKTXLoader = nullptr;
DDSLoader* gDDSLoader = nullptr;
CompressonatorLoader* gCompressonatorLoader = nullptr;
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
    gCompressonatorLoader = new CompressonatorLoader();
    gKTXLoader = new KTXLoader();
    gDDSLoader = new DDSLoader();
	gSoundsLoader = new SoundsLoader();
    //It's important to populate specific loaders first. The order matters.
	PopulateLoaderExtensions(gSoundsLoader);   
    PopulateLoaderExtensions(gCompressonatorLoader);
    PopulateLoaderExtensions(gKTXLoader);
    PopulateLoaderExtensions(gDDSLoader);
   // PopulateLoaderExtensions(gDDSLoader);
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
    std::filesystem::path cachedPath = ErmyProject::Instance().GetProjectCachePath() / (std::to_string(ID) + ".raw");

	if (std::filesystem::exists(cachedPath))
	{
		std::ifstream cached(cachedPath, std::ios::binary);
		ermy::u8 atype = 0;
		cached.read(reinterpret_cast<char*>(&atype), sizeof(ermy::u8));
		data = AssetData::CreateFromType(static_cast<AssetDataType>(atype));

        if (data)
			data->LoadFromCachedRaw(cached, source);
		return;
	}
    

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
                {
					std::ofstream cached(cachedPath, std::ios::binary);

					ermy::u8 atype = static_cast<ermy::u8>(data->GetDataType());
					cached.write(reinterpret_cast<char*>(&atype), sizeof(ermy::u8));
                    data->SaveToCachedRaw(cached);
					//save to cache
                    return;
                }                    
            }
        }


    }

    data = new BinaryAssetData();
}

Asset::Asset()
{

}

