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
		finalCL.BeginDebugScope("Render Preview");
		finalCL.BeginRenderPass(RTT, glm::vec4(0.2f, 0.3f, 0.8f, 1.0f));
		finalCL.EndRenderPass();
		finalCL.EndDebugScope();

		needUpdatePreview = false;
	}	
}


PreviewRenderer::PreviewRenderer()
{
	rendering::TextureDesc tdesc;
	tdesc.width = 256;
	tdesc.height = 256;

	RTT_Color = rendering::CreateDedicatedTexture(tdesc);
	
	rendering::RenderPassDesc rdesc;
	rdesc.colorAttachment = RTT_Color;

	RTT = rendering::CreateRenderPass(rdesc);

	previewTextureID = rendering::GetTextureDescriptor(RTT_Color);
}

PreviewRenderer::~PreviewRenderer()
{

}