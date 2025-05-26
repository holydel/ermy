#pragma once

#include <assets/asset.h>
#include <ermy_rendering.h>
#include "../compressonator_lib.h"

CMP_FORMAT CMPFormatFromErmyFormat(ermy::rendering::Format format);

// Define array sizes based on enum counts
constexpr size_t TEXTURE_PURPOSE_COUNT = 6;  // matches TexturePurpose enum count
constexpr size_t TEXTURE_COMPRESSION_COUNT = 44;  // matches TextureCompression enum count

// Declare arrays with explicit sizes
extern const char* TexturePurposeNames[TEXTURE_PURPOSE_COUNT];
extern const char* TextureCompressionNames[TEXTURE_COMPRESSION_COUNT];

class TextureAsset : public AssetData
{
	public:
	enum class TexturePurpose : ermy::u8
	{
		TP_ALBEDO //sRGB compressed
		,TP_UTILITY //linear compressed
		,TP_NORMALMAP //linear compressed RG normal (specific case but i have A LOT of normal map textures)
		,TP_HDR //linear HDR (BC6H, RGB9E5, uncompressed, ASTC HDR)
		,TP_UI //sRGB uncompressed
		,TP_SOURCE //use texture source compression or user defined compression format
		,TP_HEIGHTMAP //linear compressed RG heightmap (specific case but i have A LOT of heightmap textures)
	};

	enum class TextureCompression : ermy::u8 {
		TC_NONE,         

		TC_HDR_R11G11B10F,
		TC_HDR_RGB9E5,
		// BC Formats
		TC_BC1_SRGB,         // sRGB RGB, 4 bpp (DXT1, no alpha or 1-bit alpha)
		TC_BC1_UNORM,        // Linear RGB, 4 bpp
		TC_BC4_UNORM,        // Linear R, 4 bpp (single-channel)
		TC_BC5_UNORM,        // Linear RG, 8 bpp (e.g., normal maps)
		TC_BC6H_UNORM,       // Linear HDR RGB, 8 bpp (no alpha)
		TC_BC7_SRGB,         // sRGB RGBA, 8 bpp
		TC_BC7_UNORM,        // Linear RGBA, 8 bpp

		// ASTC LDR (Low Dynamic Range)
		TC_ASTC_LDR_4x4_SRGB_RGB,    // sRGB RGB, 8 bpp
		TC_ASTC_LDR_4x4_SRGB_RGBA,   // sRGB RGBA, 8 bpp
		TC_ASTC_LDR_4x4_UNORM_RGB,   // Linear RGB, 8 bpp
		TC_ASTC_LDR_4x4_UNORM_RGBA,  // Linear RGBA, 8 bpp
		TC_ASTC_LDR_6x6_SRGB_RGB,    // sRGB RGB, 3.56 bpp
		TC_ASTC_LDR_6x6_SRGB_RGBA,   // sRGB RGBA, 3.56 bpp
		TC_ASTC_LDR_6x6_UNORM_RGB,   // Linear RGB, 3.56 bpp
		TC_ASTC_LDR_6x6_UNORM_RGBA,  // Linear RGBA, 3.56 bpp
		TC_ASTC_LDR_8x8_SRGB_RGB,    // sRGB RGB, 2 bpp
		TC_ASTC_LDR_8x8_SRGB_RGBA,   // sRGB RGBA, 2 bpp
		TC_ASTC_LDR_8x8_UNORM_RGB,   // Linear RGB, 2 bpp
		TC_ASTC_LDR_8x8_UNORM_RGBA,  // Linear RGBA, 2 bpp

		// ASTC HDR (High Dynamic Range)
		TC_ASTC_HDR_4x4_UNORM_RGB,   // Linear HDR RGB, 8 bpp
		TC_ASTC_HDR_4x4_UNORM_RGBA,  // Linear HDR RGBA, 8 bpp
		TC_ASTC_HDR_6x6_UNORM_RGB,   // Linear HDR RGB, 3.56 bpp
		TC_ASTC_HDR_6x6_UNORM_RGBA,  // Linear HDR RGBA, 3.56 bpp
		TC_ASTC_HDR_8x8_UNORM_RGB,   // Linear HDR RGB, 2 bpp
		TC_ASTC_HDR_8x8_UNORM_RGBA,  // Linear HDR RGBA, 2 bpp

		// ETC (Ericsson Texture Compression)
		TC_ETC1_SRGB,        // sRGB RGB, 4 bpp (no alpha, legacy)
		TC_ETC2_SRGB_RGB,    // sRGB RGB, 4 bpp
		TC_ETC2_SRGB_RGBA,   // sRGB RGBA, 4 bpp
		TC_ETC2_UNORM_RGB,   // Linear RGB, 4 bpp
		TC_ETC2_UNORM_RGBA,  // Linear RGBA, 4 bpp

		// PVRTC (PowerVR Texture Compression)
		TC_PVRTC_4BPP_SRGB_RGB,   // sRGB RGB, 4 bpp
		TC_PVRTC_4BPP_SRGB_RGBA,  // sRGB RGBA, 4 bpp
		TC_PVRTC_4BPP_UNORM_RGB,  // Linear RGB, 4 bpp
		TC_PVRTC_4BPP_UNORM_RGBA, // Linear RGBA, 4 bpp
		TC_PVRTC_2BPP_SRGB_RGB,   // sRGB RGB, 2 bpp
		TC_PVRTC_2BPP_SRGB_RGBA,  // sRGB RGBA, 2 bpp
		TC_PVRTC_2BPP_UNORM_RGB,  // Linear RGB, 2 bpp
		TC_PVRTC_2BPP_UNORM_RGBA, // Linear RGBA, 2 bpp
	};

	static ermy::rendering::Format FormatFromTextureCompression(TextureCompression compression);
	private:

	float previewDX = 0.0f;
	float previewDY = 0.0f;
	float previewZoom = 1.0f;	
	bool isPreviewDragging = false;
	float oldPreviewDX = 0.0f;
	float oldPreviewDY = 0.0f;
	bool isStaticPreview = false;
	int currentArrayLevel = 0;
	int currentMip = 0;


	ermy::rendering::TextureType texType = ermy::rendering::TextureType::Tex2D;
public:
	TexturePurpose texturePurpose = TexturePurpose::TP_ALBEDO;
	TextureCompression textureCompression = TextureCompression::TC_NONE;

	//default texturePurpose / textureCompression settings
	bool regenerateMips = true;
	bool isSRGBSpace = true;

	AssetDataType GetDataType() override { return AssetDataType::Texture; }
	ermy::u32 width = 0;
	ermy::u32 height = 0;
	ermy::u32 depth = 1;
	ermy::u32 numLayers = 1;
	ermy::u32 numMips = 1;
	bool isCubemap = 0;
	bool isSparse = 0;

	ermy::rendering::Format texelSourceFormat = ermy::rendering::Format::RGBA8_SRGB;
	ermy::rendering::Format texelTargetFormat = texelSourceFormat;


	TextureAsset();
	virtual ~TextureAsset();

	void DrawPreview() override;

	void RegenerateLivePreview() override;
	std::vector<ermy::u8> GetStaticPreviewData() override;
	void RenderPreview(ermy::rendering::CommandList& cl) override;
	void RenderStaticPreview(ermy::rendering::CommandList& cl) override;
	void MouseZoom(float) override;
	void MouseDown(float normalizedX, float normalizedY, int button) override;
	void MouseUp(int button) override;
	void MouseMove(float normalizedDeltaX, float normalizedDeltaY, int button) override;
	void ResetView() override;
	
	void CompressTexture();
	void RegenerateMips();

	void SetSourceData(const ermy::u8* data, ermy::u32 dataSize);

	void UpdateTextureSettings();

	void Save(pugi::xml_node& node) override;
	void Load(pugi::xml_node& node) override;

	void LoadFromCachedRaw(std::ifstream& file, const std::filesystem::path& path) override;
	void SaveToCachedRaw(std::ofstream& file) override;
};