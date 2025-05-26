#pragma once
#include "assets_loader.h"
#include "font_asset.h"
#include <vector>

namespace editor
{
	namespace asset
	{
		namespace loader
		{
			class FontLoader : public AssetsLoader
			{
			public:
				FontLoader();
				virtual ~FontLoader();

				AssetData* Load(const std::filesystem::path& path) override;
				const std::vector<std::string>& SupportedExtensions() override;
			};
		}
	}
}