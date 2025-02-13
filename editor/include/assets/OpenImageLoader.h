#pragma once
#include "assets_loader.h"

namespace editor
{
	namespace asset
	{
		namespace loader
		{
			class OpenImageLoader : public AssetsLoader
			{
			public:
				OpenImageLoader();
				virtual ~OpenImageLoader();

				Asset* Load(const std::filesystem::path& path) override;
				const std::vector<std::string>& SupportedExtensions() override;
			};
		}
	}
}