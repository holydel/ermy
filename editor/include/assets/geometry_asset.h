#pragma once

#include <assets/asset.h>

class GeometryAsset : public AssetData
{
public:
	struct GeometryAssetMeta : public AssetData::MetaData
	{

	};

	GeometryAsset();
	virtual ~GeometryAsset();
};