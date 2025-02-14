#pragma once
#include "assets_loader.h"

namespace editor
{
	namespace asset
	{
		namespace loader
		{
			class FFMpegLoader : public AssetsLoader
			{
			public:
				FFMpegLoader();
				virtual ~FFMpegLoader();

				AssetData* Load(const std::filesystem::path& path) override;
				const std::vector<std::string>& SupportedExtensions() override;
			};
		}
	}
}