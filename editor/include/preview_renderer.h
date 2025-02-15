#pragma once
#include <ermy_rendering.h>
#include <ermy_commandlist.h>
#include <imgui.h>

class PreviewRenderer
{	
	ermy::rendering::RenderPassID RTT;

	ermy::rendering::TextureID RTT_Color;
	ermy::rendering::TextureID RTT_Depth;
	PreviewRenderer();
	~PreviewRenderer();

	ImTextureID previewTextureID;
	bool needUpdatePreview = true;
public:
	static PreviewRenderer& Instance();

	void RenderPreview(ermy::rendering::CommandList& finalCL);
	ImTextureID GetPreviewTexture()
	{
		needUpdatePreview = true;
		return previewTextureID;
	}
};