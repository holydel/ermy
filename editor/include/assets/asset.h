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
#include <span>

enum class AssetType : ermy::u8
{
	UNKNOWN,
	AUTO,
	Folder,
	General,
};

enum class AssetDataType : ermy::u8
{
	UNKNOWN,
	Texture,
	Material,
	Geometry,
	Sound,
	Video,
};

enum class AssetLoaderType : ermy::u8
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
	ImTextureID assetPreviewTexLive = 0;

	///uncompressed data from asset like RGBA8 / RGBA16F / BC7 for uncompressed/compressed textures
	///or raw audio data for sound
	///or 32bit float vertex data for geometry
	std::vector<ermy::u8> rawData;

	/// <summary>
	/// lossy processed data for assets, like BC1/BC6 for textures (already compressed textures use raw data)
	/// or half float vertex data for geometry
	/// </summary>
	std::vector<ermy::u8> compressedData;

	/// <summary>
	/// lossless packed compressed data. LZ4, GDeflate, Brotli, etc. Direct write to PAK file
	/// </summary>
	std::vector<ermy::u8> packedData;
public:
	static AssetData* CreateFromType(AssetDataType type);
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

	virtual void RegenerateLivePreview() {}
	virtual std::vector<ermy::u8> GetStaticPreviewData() { return std::move(std::vector<ermy::u8>()); };

	virtual void Save(pugi::xml_node& node);
	virtual void Load(pugi::xml_node& node) {}
	virtual void LoadFromCachedRaw(std::ifstream& file, const std::filesystem::path& path) {}
	virtual void SaveToCachedRaw(std::ofstream& file) {}
	virtual void ProcessAssetToPak() {};

	std::span<ermy::u8> GetPakData()	
	{
		if(!packedData.empty())
			return packedData;

		if (!compressedData.empty())
			return compressedData;

		return rawData;
	}
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
	ermy::u64 ID = 0;
	void CalculateAssetName();

	ermy::rendering::TextureID previewTextureStatic;
	ImTextureID assetPreviewTexStatic = 0;
	bool needRegenerateStaticPreview = true;
	void RegenerateStaticPreviewTexture();
public:
	ImTextureID GetAssetPreviewStatic()
	{		
		RegenerateStaticPreviewTexture();
		return assetPreviewTexStatic;
	}

	std::filesystem::path GetSourcePath() const
	{
		return source;
	}

	ermy::u64 GetID() const
	{
		return ID;
	}

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