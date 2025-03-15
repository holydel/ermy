#pragma once
#include "assets_loader.h"

namespace editor
{
	namespace asset
	{
		namespace loader
		{
			class CompressonatorLoader : public AssetsLoader
			{
			public:
				CompressonatorLoader();
				virtual ~CompressonatorLoader();

				AssetData* Load(const std::filesystem::path& path) override;
				const std::vector<std::string>& SupportedExtensions() override;
			};
		}
	}
}