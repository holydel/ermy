#pragma once
#include "asset.h"
#include <filesystem>
#include <string>
#include <vector>

namespace editor
{
	namespace asset
	{
		namespace loader
		{
			/// <summary>
			/// Load uncompressed raw data to AssetData
			/// </summary>
			class AssetsLoader
			{
			public:
				AssetsLoader() = default;
				virtual ~AssetsLoader() = default;

				virtual AssetData* Load(const std::filesystem::path& path) = 0;
				virtual const std::vector<std::string>& SupportedExtensions() = 0;
				static bool Initialize();
				static bool Shutdown();
			};
		}
	}
}