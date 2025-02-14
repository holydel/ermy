#pragma once

#include <ermy_api.h>
#include <string>
#include <filesystem>
#include <vector>
#include "props.h"
#include <imgui.h>

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

class AssetData
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

	virtual void DrawPreview() {};
protected:
	MetaData* metaData = nullptr;
};

class Asset : public Props
{
protected:
	std::filesystem::path source = "";
	std::filesystem::path intermediate = "";
	std::string name = "";
	AssetData* data = nullptr;
	ermy::u64 fileSize = 0;

	std::string assetName = "";
	ermy::u8 assetNameRows = 0;
public:
	ermy::u64 ID = 0;
	AssetData* GetData()
	{
		return data;
	}

	void Import();

	Asset();
	~Asset() = default;

	virtual AssetType GetType() { return AssetType::UNKNOWN; }

	static Asset* CreateFromPath(const std::filesystem::path path);
	const char* NameCStr() const
	{
		return name.c_str();
	}
	const char* AssetNameCStr() const
	{
		return assetName.c_str();
	}

	ermy::u8 GetAssetNameRows() const
	{
		return assetNameRows;
	}	 

	void DrawProps();

	ermy::u64 FileSize()
	{
		return fileSize;
	}
};

class AssetFolder : public Asset
{
	class FolderData : public AssetData
	{
		friend AssetFolder;

		ermy::u32 numFiles = 0;
		ermy::u64 totalSize = 0;

		void DrawPreview() override;
	} folderData;
public:
	AssetFolder()
	{
		data = &folderData;
	}
	virtual ~AssetFolder() = default;
	std::vector<Asset*> content;
	std::vector<AssetFolder*> subdirectories;
	AssetType GetType() override { return AssetType::Folder; }

	void Scan(const std::filesystem::path& path);
};

class BinaryAssetData : public AssetData
{
	void* binaryBlob = nullptr;
	ermy::u64 blobSize = 0;
public:
	void DrawPreview() override;
};