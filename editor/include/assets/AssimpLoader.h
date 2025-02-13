#pragma once
#include "assets_loader.h"

namespace editor
{
	namespace asset
	{
		namespace loader
		{
			class AssimpLoader : public AssetsLoader
			{
			public:
				AssimpLoader();
				virtual ~AssimpLoader();

				Asset* Load(const std::filesystem::path& path) override;		
				const std::vector<std::string>& SupportedExtensions() override;
			};
		}
	}
}