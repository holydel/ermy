#pragma once

#include <ermy_api.h>
#include <string>
#include <filesystem>
#include <vector>
#include "props.h"
#include <imgui.h>
#include <ermy_rendering.h>
#include "preview_props.h"
#include <pugixml.hpp>

enum class AssetType
{
	UNKNOWN,
	AUTO,
	Folder,
	General,
};

enum class AssetDataType
{
	UNKNOWN,
	Texture,
	Material,
	Geometry,
	Sound,
	Video,
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

class AssetData : public PreviewProps
{
protected:
	ermy::rendering::TextureID previewTextureLive;
	ermy::rendering::TextureID previewTextureStatic;
	ImTextureID assetPreviewTexLive = 0;
	ImTextureID assetPreviewTexStatic = 0;
public:
	
	virtual AssetDataType GetDataType() { return AssetDataType::UNKNOWN; }
	struct MetaData
	{
	public:
		MetaData() = default;
		virtual ~MetaData() = default;
	};

	AssetData() = default;
	~AssetData() = default;

	virtual void DrawPreview() {};
	ermy::rendering::TextureID GetPreviewTexture()
	{
		return previewTextureLive;
	}
	ImTextureID GetAssetPreviewStatic()
	{
		return assetPreviewTexStatic;
	}
	virtual void RegeneratePreview() {}
protected:
	MetaData* metaData = nullptr;
};

class Asset : public Props
{
	bool includeInPAK = false;
protected:
	std::filesystem::path source = "";
	std::filesystem::path intermediate = "";
	std::string name = "";
	AssetData* data = nullptr;
	ermy::u64 fileSize = 0;
	std::filesystem::file_time_type	lastWriteTime = {};
	std::string assetName = "";
	ermy::u8 assetNameRows = 0;
	void CalculateAssetName();
public:
	std::filesystem::path GetSourcePath() const
	{
		return source;
	}

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

	ermy::u64 FileSize() const
	{
		return fileSize;
	}

	bool NeedToIncludeInPAK() const
	{
		return includeInPAK;
	}

	virtual pugi::xml_node Save(pugi::xml_node& node, const std::filesystem::path& rootPath);
	virtual void Load(pugi::xml_node& node, const std::filesystem::path& currentPath);
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

	virtual pugi::xml_node Save(pugi::xml_node& node, const std::filesystem::path& rootPath) override;
	virtual void Load(pugi::xml_node& node, const std::filesystem::path& currentPath) override;
};

class BinaryAssetData : public AssetData
{
	void* binaryBlob = nullptr;
	ermy::u64 blobSize = 0;
public:
	void DrawPreview() override;
};