#pragma once

#include <assets/asset.h>
#include <ermy_rendering.h>

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
	ermy::rendering::TextureType texType = ermy::rendering::TextureType::Tex2D;
public:
	ermy::u32 width = 0;
	ermy::u32 height = 0;
	ermy::u32 depth = 1;
	ermy::u32 numLayers = 1;
	ermy::u32 numMips = 1;
	ermy::u32 dataSize = 1;
	ermy::u8 numChannels = 4;
	bool isCubemap = 0;
	bool isSparse = 0;
	void* data = nullptr;
	ermy::rendering::Format texelFormat;

	TextureAsset();
	virtual ~TextureAsset();

	void DrawPreview() override;

	void RegeneratePreview() override;
	void RenderPreview(ermy::rendering::CommandList& cl) override;
	void RenderStaticPreview(ermy::rendering::CommandList& cl) override;
	void MouseZoom(float) override;
	void MouseDown(float normalizedX, float normalizedY) override;
	void MouseUp() override;
	void MouseMove(float normalizedDeltaX, float normalizedDeltaY) override;
	void ResetView() override;
};