#pragma once

#include <ermy_api.h>
#include <string>
#include <filesystem>
#include <vector>

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
	AssetLoaderType importerToLoad = AssetLoaderType::UNKNOWN;
public:
	void Import() {};

	Asset() = default;
	~Asset() = default;

	virtual AssetType GetType() { return AssetType::Folder; }

	static Asset* CreateFromPath(const std::filesystem::path path);
};

class AssetFolder : public Asset
{
public:
	std::vector<Asset*> content;
	AssetType GetType() override { return AssetType::UNKNOWN; }

	void Scan(const std::filesystem::path& path);
};

struct FormatExtensionInfo
{
	std::string ext;
	AssetType atype;
};