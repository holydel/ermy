#pragma once
#include "assets_loader.h"

namespace editor
{
	namespace asset
	{
		namespace loader
		{
			class KTXLoader : public AssetsLoader
			{
			public:
				KTXLoader();
				virtual ~KTXLoader();

				Asset* Load(const std::filesystem::path& path) override;
				const std::vector<std::string>& SupportedExtensions() override;
			};
		}
	}
}