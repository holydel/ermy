#include <assets/assets_importer.h>
#include <assets/geometry_asset.h>
#include <assets/sound_asset.h>
#include <assets/texture_asset.h>
#include <assets/video_asset.h>


void assets_importer::Initialize()
{
	auto textureExts = TextureAsset::Initialize();
	auto soundExts = SoundAsset::Initialize();
	auto geometryExts = GeometryAsset::Initialize();
	auto videoExts = VideoTextureAsset::Initialize();

	printf("foo");
}

void assets_importer::Shutdown()
{
	VideoTextureAsset::Initialize();
	GeometryAsset::Initialize();
	SoundAsset::Initialize();
	TextureAsset::Initialize();
}

Asset* assets_importer::Import(const char* filename, AssetType astype)
{
	switch (astype)
	{
		case AssetType::Geometry:
			return new GeometryAsset(filename);
		case AssetType::Texture:
			return new TextureAsset(filename);
		case AssetType::VideoTexture:
			return new VideoTextureAsset(filename);
		case AssetType::Sound:
			return new SoundAsset(filename);
	}
	
	return nullptr;
}