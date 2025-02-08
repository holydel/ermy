#pragma once
#include "asset.h"

namespace assets_importer
{
	void Initialize();
	void Shutdown();
	Asset* Import(const char* filename, AssetType astype = AssetType::AUTO);
};