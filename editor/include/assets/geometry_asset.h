#pragma once

#include <assets/asset.h>

class GeometryAsset : public Asset
{
public:
	struct GeometryAssetMeta : public Asset::MetaData
	{

	};

	GeometryAsset(const char* filepath);
	virtual ~GeometryAsset();

	static std::vector<std::string> Initialize();
	static void Shutdown();
};