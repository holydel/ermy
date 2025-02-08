#pragma once

#include <ermy_api.h>
#include <string>
#include <filesystem>

class Asset
{
public:
	struct MetaData
	{
	public:
		MetaData() = default;
		virtual ~MetaData() = default;
	};
protected:
	std::filesystem::path source = "";
	std::filesystem::path intermediate = "";

	MetaData* metaData = nullptr;
public:
	void Import() {};

	Asset() = default;
	~Asset() = default;
};

enum class AssetType
{
	AUTO,

	Texture,
	Geometry,
	Sound,
	VideoTexture
};


struct FormatExtensionInfo
{
	std::string ext;
	AssetType atype;
};