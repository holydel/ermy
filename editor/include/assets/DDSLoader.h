#pragma once
#include "assets_loader.h"

namespace editor
{
	namespace asset
	{
		namespace loader
		{
			class DDSLoader : public AssetsLoader
			{
			public:
				DDSLoader();
				virtual ~DDSLoader();

				AssetData* Load(const std::filesystem::path& path) override;
				const std::vector<std::string>& SupportedExtensions() override;
			};
		}
	}
}