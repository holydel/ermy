#pragma once
#include <ermy_rendering.h>
#include <ermy_commandlist.h>
#include <imgui.h>
#include "preview_props.h"
#include "assets/asset.h"
#include <queue>
#include <functional>

class PreviewRenderer
{	
	ermy::rendering::RenderPassID RTT;

	ermy::rendering::TextureID RTT_Color;
	ermy::rendering::TextureID RTT_Depth;
	PreviewRenderer();
	~PreviewRenderer();

	ImTextureID previewTextureID;
	ermy::u64 previewTextureIDStatic;
	bool needUpdatePreview = true;

	PreviewProps* currentProps = nullptr;

	ermy::rendering::RenderPassID RTT_Static;

	ermy::rendering::TextureID RTT_ColorStaticDoubleRes;
	ermy::rendering::TextureID RTT_DepthStaticDoubleRes;
	ermy::rendering::TextureID RTT_ColorStaticFinal;
public:
	static PreviewRenderer& Instance();

	void RenderPreview(ermy::rendering::CommandList& finalCL);
	ImTextureID GetPreviewTexture()
	{
		needUpdatePreview = true;
		return previewTextureID;
	}

	void SetPreviewProps(PreviewProps* props)
	{
		currentProps = props;
	}

	ermy::rendering::RenderPassID GetRTT()
	{
		return RTT;
	}
	ermy::rendering::RenderPassID GetStaticRTT()
	{
		return RTT_Static;
	}
	ermy::rendering::TextureID GetStaticTexture()
	{
		return RTT_ColorStaticDoubleRes;
	}

	void MouseZoom(float);
	void MouseDown(float normalizedX, float normalizedY, int button);
	void MouseUp(int button);
	void MouseMove(float normalizedDeltaX, float normalizedDeltaY, int button);
	void ResetView();

	void UpdateStaticPreviews()
	{

	}

	std::vector<ermy::u8> GetPreviewDataBC1(std::function<void(ermy::rendering::CommandList&)> renderFunc);
};