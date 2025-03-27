#include <preview_renderer.h>
#include <imgui.h>
#include <compressonator_lib.h>
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

void PreviewRenderer::MouseDown(float normalizedX, float normalizedY, int button)
{
	if (currentProps)
		currentProps->MouseDown(normalizedX, normalizedY, button);
}

void PreviewRenderer::MouseUp(int button)
{
	if (currentProps)
		currentProps->MouseUp(button);
}

void PreviewRenderer::MouseMove(float normalizedDeltaX, float normalizedDeltaY, int button)
{
	if (currentProps)
		currentProps->MouseMove(normalizedDeltaX, normalizedDeltaY,button);
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
		tdesc_color.texelSourceFormat = rendering::Format::RGBA8_UNORM;
		tdesc_color.debugName = "Live Preview Color RTT";
		
		rendering::TextureDesc tdesc_depth;
		tdesc_depth.width = 1024;
		tdesc_depth.height = 1024;
		tdesc_depth.texelSourceFormat = rendering::Format::D32F;
		tdesc_depth.debugName = "Live Preview Depth RTT";

		RTT_Color = rendering::CreateDedicatedTexture(tdesc_color);
		RTT_Depth = rendering::CreateDedicatedTexture(tdesc_depth);


		rendering::RenderPassDesc rdesc;
		rdesc.colorAttachment = RTT_Color;
		rdesc.depthStencilAttachment = RTT_Depth;
		rdesc.debugName = "Live Preview RTT";
		RTT = rendering::CreateRenderPass(rdesc);

		previewTextureID = rendering::GetTextureDescriptor(RTT_Color);
	}


	{
		rendering::TextureDesc tdesc;
		tdesc.width = 256;
		tdesc.height = 256;
		tdesc.debugName = "Static Preview Color RTT DoubleRes";
		RTT_ColorStaticDoubleRes = rendering::CreateDedicatedTexture(tdesc);

		rendering::TextureDesc tdescFinal;
		tdescFinal.width = 128;
		tdescFinal.height = 128;
		tdescFinal.debugName = "Static Preview Color RTT Final";
		RTT_ColorStaticFinal = rendering::CreateDedicatedTexture(tdescFinal);

		rendering::TextureDesc tdesc_depth;
		tdesc_depth.width = 256;
		tdesc_depth.height = 256;
		tdesc_depth.texelSourceFormat = rendering::Format::D32F;
		tdesc_depth.debugName = "Static Preview Depth RTT DoubleRes";

		RTT_DepthStaticDoubleRes = rendering::CreateDedicatedTexture(tdesc_depth);

		rendering::RenderPassDesc rdesc;
		rdesc.colorAttachment = RTT_ColorStaticDoubleRes;
		rdesc.depthStencilAttachment = RTT_DepthStaticDoubleRes;
		rdesc.debugName = "Static Preview RTT DoubleRes";
		RTT_Static = rendering::CreateRenderPass(rdesc);

		previewTextureIDStatic = rendering::GetTextureDescriptor(RTT_ColorStaticDoubleRes);
	}
}

PreviewRenderer::~PreviewRenderer()
{

}

std::vector<ermy::u8> PreviewRenderer::GetPreviewDataBC1(std::function<void(ermy::rendering::CommandList&)> renderFunc)
{
	rendering::OneTimeSubmitCommandList otscl = rendering::OneTimeSubmitCommandList::Allocate();
	auto cl = otscl.GetCL();

	cl.BeginRenderPass(RTT_Static);

	renderFunc(cl);

	cl.EndRenderPass();

	cl.BlitTexture(RTT_ColorStaticDoubleRes, RTT_ColorStaticFinal);

	otscl.Submit();
	otscl.WaitForCompletion();

	std::vector<u8> dataVec(128 * 128 * 4);

	rendering::ReadbackTexture(RTT_ColorStaticFinal, dataVec.data());

	std::vector<u8> bc1_data(128 * 128 / 2);

	CMP_Texture srcTexture;
	srcTexture.dwSize = sizeof(CMP_Texture);
	srcTexture.dwWidth = 128;
	srcTexture.dwHeight = 128;
	srcTexture.dwPitch = 128 * 4;
	srcTexture.format = CMP_FORMAT_RGBA_8888;
	srcTexture.dwDataSize = 128 * 128 * 4;
	srcTexture.pData = dataVec.data();

	CMP_Texture destTexture;
	destTexture.dwSize = sizeof(CMP_Texture);
	destTexture.dwWidth = 128;
	destTexture.dwHeight = 128;
	destTexture.dwPitch = 128;
	destTexture.format = CMP_FORMAT_BC1;
	destTexture.dwDataSize = 128 * 128 / 2;
	destTexture.pData = bc1_data.data();

	CMP_CompressOptions options;
	options.dwSize = sizeof(CMP_CompressOptions);
	options.fquality = 1.0f;
	//options.dwnumThreads = 1;

	CMP_ConvertTexture(&srcTexture, &destTexture, &options, nullptr);

	return bc1_data;
}