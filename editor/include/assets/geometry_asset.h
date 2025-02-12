#pragma once

#include <assets/asset.h>

class GeometryAsset : public Asset
{
public:
	struct GeometryAssetMeta : public Asset::MetaData
	{

	};

	GeometryAsset();
	virtual ~GeometryAsset();
};