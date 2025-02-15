#include <preview_renderer.h>
#include <imgui.h>
using namespace ermy;

PreviewRenderer& PreviewRenderer::Instance()
{
	static PreviewRenderer lInstance;
	return lInstance;
	// TODO: insert return statement here
}

void PreviewRenderer::RenderPreview(ermy::rendering::CommandList& finalCL)
{
	if (needUpdatePreview)
	{
		static float a = 0.0f;
		a += 0.01f;

		finalCL.BeginDebugScope("Render Preview");
		finalCL.BeginRenderPass(RTT, glm::vec4(0.2f, sin(a) * 0.5f + 0.5f, 0.8f, 1.0f));

		if (currentProps)
			currentProps->RenderPreview(finalCL);

		finalCL.EndRenderPass();
		finalCL.EndDebugScope();

		needUpdatePreview = false;
	}	
}


PreviewRenderer::PreviewRenderer()
{
	rendering::TextureDesc tdesc;
	tdesc.width = 512;
	tdesc.height = 512;

	RTT_Color = rendering::CreateDedicatedTexture(tdesc);
	
	rendering::RenderPassDesc rdesc;
	rdesc.colorAttachment = RTT_Color;

	RTT = rendering::CreateRenderPass(rdesc);

	previewTextureID = rendering::GetTextureDescriptor(RTT_Color);
}

PreviewRenderer::~PreviewRenderer()
{

}