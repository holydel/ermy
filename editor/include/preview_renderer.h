#pragma once
#include <ermy_rendering.h>
#include <ermy_commandlist.h>
#include <imgui.h>
#include "preview_props.h"

class PreviewRenderer
{	
	ermy::rendering::RenderPassID RTT;

	ermy::rendering::TextureID RTT_Color;
	ermy::rendering::TextureID RTT_Depth;
	PreviewRenderer();
	~PreviewRenderer();

	ImTextureID previewTextureID;
	bool needUpdatePreview = true;

	PreviewProps* currentProps = nullptr;
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

	void MouseZoom(float);
	void MouseDown(float normalizedX, float normalizedY);
	void MouseUp();
	void MouseMove(float normalizedDeltaX, float normalizedDeltaY);
	void ResetView();
};