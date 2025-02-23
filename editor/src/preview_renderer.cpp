#include <preview_renderer.h>
#include <imgui.h>
using namespace ermy;

PreviewRenderer& PreviewRenderer::Instance()
{
	static PreviewRenderer lInstance;
	return lInstance;
	// TODO: insert return statement here
}
void PreviewRenderer::ResetView()
{
	if (currentProps)
		currentProps->ResetView();
}

void PreviewRenderer::MouseZoom(float value)
{
	if (currentProps)
		currentProps->MouseZoom(value);
}

void PreviewRenderer::MouseDown(float normalizedX, float normalizedY)
{
	if (currentProps)
		currentProps->MouseDown(normalizedX, normalizedY);
}

void PreviewRenderer::MouseUp()
{
	if (currentProps)
		currentProps->MouseUp();
}

void PreviewRenderer::MouseMove(float normalizedDeltaX, float normalizedDeltaY)
{
	if (currentProps)
		currentProps->MouseMove(normalizedDeltaX, normalizedDeltaY);
}

void PreviewRenderer::RenderPreview(ermy::rendering::CommandList& finalCL)
{
	if (needUpdatePreview)
	{
		static float a = 0.0f;
		a += 0.01f;

		finalCL.BeginDebugScope("Render Preview");
		finalCL.BeginRenderPass(RTT, glm::vec4(0,0,0,0));

		if (currentProps)
			currentProps->RenderPreview(finalCL);

		finalCL.EndRenderPass();
		finalCL.EndDebugScope();

		needUpdatePreview = false;
	}	

	if (!staticPreviewList.empty())
	{
		finalCL.BeginDebugScope("Render Static Preview");

		while (!staticPreviewList.empty())
		{
			AssetData* toGenPreview = staticPreviewList.front();
			toGenPreview->RenderStaticPreview(finalCL);
			staticPreviewList.pop();
		}

		finalCL.EndDebugScope();

	}
}


PreviewRenderer::PreviewRenderer()
{
	{
		rendering::TextureDesc tdesc_color;
		tdesc_color.width = 1024;
		tdesc_color.height = 1024;
		tdesc_color.texelFormat = rendering::Format::RGBA8_UNORM;
		rendering::TextureDesc tdesc_depth;
		tdesc_depth.width = 1024;
		tdesc_depth.height = 1024;
		tdesc_depth.texelFormat = rendering::Format::D32F;

		RTT_Color = rendering::CreateDedicatedTexture(tdesc_color);
		RTT_Depth = rendering::CreateDedicatedTexture(tdesc_depth);


		rendering::RenderPassDesc rdesc;
		rdesc.colorAttachment = RTT_Color;
		rdesc.depthStencilAttachment = RTT_Depth;

		RTT = rendering::CreateRenderPass(rdesc);

		previewTextureID = rendering::GetTextureDescriptor(RTT_Color);
	}


	{
		rendering::TextureDesc tdesc;
		tdesc.width = 256;
		tdesc.height = 256;

		RTT_ColorStaticDoubleRes = rendering::CreateDedicatedTexture(tdesc);

		rendering::TextureDesc tdesc_depth;
		tdesc_depth.width = 256;
		tdesc_depth.height = 256;
		tdesc_depth.texelFormat = rendering::Format::D32F;

		RTT_DepthStaticDoubleRes = rendering::CreateDedicatedTexture(tdesc_depth);

		rendering::RenderPassDesc rdesc;
		rdesc.colorAttachment = RTT_ColorStaticDoubleRes;
		rdesc.depthStencilAttachment = RTT_DepthStaticDoubleRes;

		RTT_Static = rendering::CreateRenderPass(rdesc);

		previewTextureIDStatic = rendering::GetTextureDescriptor(RTT_ColorStaticDoubleRes);
	}
}

PreviewRenderer::~PreviewRenderer()
{

}