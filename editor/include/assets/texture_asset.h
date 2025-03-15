#pragma once

#include <assets/asset.h>
#include <ermy_rendering.h>
#include "../compressonator_lib.h"

CMP_FORMAT CMPFormatFromErmyFormat(ermy::rendering::Format format);

class TextureAsset : public AssetData
{
	float previewDX = 0.0f;
	float previewDY = 0.0f;
	float previewZoom = 1.0f;	
	bool isPreviewDragging = false;
	float oldPreviewDX = 0.0f;
	float oldPreviewDY = 0.0f;
	bool isStaticPreview = false;
	int currentArrayLevel = 0;
	int currentMip = 0;
	bool regenerateMips = false;
	bool isSRGBSpace = true;

	ermy::rendering::TextureType texType = ermy::rendering::TextureType::Tex2D;

	enum class TexturePurpose : ermy::u8
	{
		TP_ALBEDO //sRGB compressed
		,TP_UTILITY //linear compressed
		,TP_NORMALMAP //linear compressed RG normal (specific case but i have A LOT of normal map textures)
		,TP_HDR //linear HDR (BC6H, RGB9E5, uncompressed, ASTC HDR)
		,TP_UI //sRGB uncompressed
		,TP_SOURCE //use texture source compression or user defined compression format
	};

	enum class TextureCompression : ermy::u8 {
		TC_NONE,         

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

	inline static const char* TexturePurposeNames[] = { "Albedo (sRGB Compressed)",
	"Utility (Linear Compressed)",
	"Normal Map (Linear RG Compressed)",
	"HDR (Linear)",
	"UI (sRGB Uncompressed)",
	"Source (User-Defined)" };


	inline static const char* TextureCompressionNames[] = {
	"None (source)",

	// BC
	"BC1 (sRGB RGB, 4 bpp)",
	"BC1 (Linear RGB, 4 bpp)",
	"BC4 (Linear R, 4 bpp)",
	"BC5 (Linear RG, 8 bpp)",
	"BC6H (HDR RGB, 8 bpp)",
	"BC7 (sRGB RGBA, 8 bpp)",
	"BC7 (Linear RGBA, 8 bpp)",

	// ASTC LDR
	"ASTC LDR 4x4 (sRGB RGB, 8 bpp)",
	"ASTC LDR 4x4 (sRGB RGBA, 8 bpp)",
	"ASTC LDR 4x4 (Linear RGB, 8 bpp)",
	"ASTC LDR 4x4 (Linear RGBA, 8 bpp)",
	"ASTC LDR 6x6 (sRGB RGB, 3.56 bpp)",
	"ASTC LDR 6x6 (sRGB RGBA, 3.56 bpp)",
	"ASTC LDR 6x6 (Linear RGB, 3.56 bpp)",
	"ASTC LDR 6x6 (Linear RGBA, 3.56 bpp)",
	"ASTC LDR 8x8 (sRGB RGB, 2 bpp)",
	"ASTC LDR 8x8 (sRGB RGBA, 2 bpp)",
	"ASTC LDR 8x8 (Linear RGB, 2 bpp)",
	"ASTC LDR 8x8 (Linear RGBA, 2 bpp)",

	// ASTC HDR
	"ASTC HDR 4x4 (HDR RGB, 8 bpp)",
	"ASTC HDR 4x4 (HDR RGBA, 8 bpp)",
	"ASTC HDR 6x6 (HDR RGB, 3.56 bpp)",
	"ASTC HDR 6x6 (HDR RGBA, 3.56 bpp)",
	"ASTC HDR 8x8 (HDR RGB, 2 bpp)",
	"ASTC HDR 8x8 (HDR RGBA, 2 bpp)",

	// ETC
	"ETC1 (sRGB RGB, 4 bpp)",
	"ETC2 (sRGB RGB, 4 bpp)",
	"ETC2 (sRGB RGBA, 4 bpp)",
	"ETC2 (Linear RGB, 4 bpp)",
	"ETC2 (Linear RGBA, 4 bpp)",

	// PVRTC
	"PVRTC 4bpp (sRGB RGB)",
	"PVRTC 4bpp (sRGB RGBA)",
	"PVRTC 4bpp (Linear RGB)",
	"PVRTC 4bpp (Linear RGBA)",
	"PVRTC 2bpp (sRGB RGB)",
	"PVRTC 2bpp (sRGB RGBA)",
	"PVRTC 2bpp (Linear RGB)",
	"PVRTC 2bpp (Linear RGBA)",
	};

	CMP_MipSet sourceMipSet = {};
	CMP_MipSet targetMipSet = {};
public:
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

	TexturePurpose texturePurpose = TexturePurpose::TP_ALBEDO;
	TextureCompression textureCompression = TextureCompression::TC_NONE;
	TextureAsset();
	virtual ~TextureAsset();

	void DrawPreview() override;

	void RegeneratePreview() override;
	void RenderPreview(ermy::rendering::CommandList& cl) override;
	void RenderStaticPreview(ermy::rendering::CommandList& cl) override;
	void MouseZoom(float) override;
	void MouseDown(float normalizedX, float normalizedY, int button) override;
	void MouseUp(int button) override;
	void MouseMove(float normalizedDeltaX, float normalizedDeltaY, int button) override;
	void ResetView() override;
	void CompressTexture();
	void SetSourceData(const ermy::u8* data, ermy::u32 dataSize);

	CMP_MipSet& GetSourceMipSet()
	{
		return sourceMipSet;
	}

	CMP_MipSet& GetTargetMipSet()
	{
		if (texelSourceFormat == texelTargetFormat)
			return sourceMipSet;

		if (targetMipSet.m_format != CMPFormatFromErmyFormat(texelTargetFormat))
			CompressTexture();

		return targetMipSet;
	}

	void UpdateTextureSettings();
};