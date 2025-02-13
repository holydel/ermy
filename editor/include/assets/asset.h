#pragma once

#include <ermy_api.h>
#include <string>
#include <filesystem>
#include <vector>
#include "props.h"

enum class AssetType
{
	UNKNOWN,
	AUTO,
	Folder,
	Texture,
	Geometry,
	Sound,
	VideoTexture
};

enum class AssetLoaderType
{
	UNKNOWN,
	FFMPEG,
	OpenImage,
	Assimp,
	KTX,
	Miniaudio
};

class AssetData : public Props
{
public:
	struct MetaData
	{
	public:
		MetaData() = default;
		virtual ~MetaData() = default;
	};

	AssetData() = default;
	~AssetData() = default;
protected:
	MetaData* metaData = nullptr;
};

class Asset
{
protected:
	std::filesystem::path source = "";
	std::filesystem::path intermediate = "";
	std::string name = "";
	AssetData* data = nullptr;
public:
	AssetData* GetData()
	{
		return data;
	}

	void Import();

	Asset() = default;
	~Asset() = default;

	virtual AssetType GetType() { return AssetType::UNKNOWN; }

	static Asset* CreateFromPath(const std::filesystem::path path);
	const char* NameCStr() const
	{
		return name.c_str();
	}
};

class AssetFolder : public Asset
{
public:
	std::vector<Asset*> content;
	std::vector<AssetFolder*> subdirectories;
	AssetType GetType() override { return AssetType::Folder; }

	void Scan(const std::filesystem::path& path);
};