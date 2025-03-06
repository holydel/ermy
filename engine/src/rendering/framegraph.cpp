#include "framegraph.h"
#include "framegraph_interface.h"
#include "imgui/imgui_interface.h"
#include "application.h"
#include "swapchain_interface.h"
#include "graphics/canvas_interface.h"
#include "rendering.h"
#include "scene/scene.h"
#include "xr/xr_interface.h"

using namespace ermy::rendering;

BufferID gFrameConstants;

void framegraph::Initialize(ermy::u8 numFrames)
{
	framegraph_interface::Initialize(numFrames);

	ermy::rendering::BufferDesc frameConstDesc;
	frameConstDesc.debugName = "Frame Constants";
	frameConstDesc.size = sizeof(ErmyFrame);
	frameConstDesc.usage = BufferUsage::Uniform;

	gFrameConstants = CreateDedicatedBuffer(frameConstDesc);

	canvas_interface::Initialize();
	imgui_interface::Initialize();
}

void framegraph::Shutdown()
{
	canvas_interface::Shutdown();
	framegraph_interface::Shutdown();
}

void framegraph::Process()
{
	auto& app = GetApplication();

	xr_interface::WaitFrame();

	app.OnUpdate();

	void* finalCmdList = framegraph_interface::BeginFrame();

	xr_interface::AcquireImage();

	ermy::rendering::CommandList clist(finalCmdList);
	
	int finalPassWidth = swapchain::GetWidth();
	int finalPassHeight = swapchain::GetHeight();

	gErmyFrameConstants.canvasRepSizeHalf = glm::vec2(2.0 / float(finalPassWidth), 2.0 / float(finalPassHeight));
	gErmyFrameConstants.canvasClearColor.r = canvas_interface::BgColor[0];
	gErmyFrameConstants.canvasClearColor.g = canvas_interface::BgColor[1];
	gErmyFrameConstants.canvasClearColor.b = canvas_interface::BgColor[2];
	gErmyFrameConstants.canvasClearColor.a = canvas_interface::BgColor[3];

	scene_internal::UpdateUniforms();
	UpdateBufferData(gFrameConstants, &gErmyFrameConstants);
	app.OnBeginFrame(clist);

	//xr_interface::Process();

	bool shouldXRendering = xr_interface::BeginXRFinalRenderPass(clist);

	if (shouldXRendering)
	{
		scene_internal::Render(clist, true);
	}

	xr_interface::EndXRFinalRenderPass(clist);


	framegraph_interface::BeginFinalRenderPass();

	clist.SetViewport(0, 0, finalPassWidth, finalPassHeight);
	clist.SetScissor(0, 0, finalPassWidth, finalPassHeight);

	imgui_interface::BeginFrame(finalCmdList);
	//render scene
	//scene_internal::Render(clist,false);
	//render canvas
	canvas_interface::SetCommandList(&clist);

	app.OnBeginFinalPass(clist);
	app.OnIMGUI();
	imgui_interface::EndFrame(finalCmdList);

	framegraph_interface::EndFinalRenderPass();

	app.OnEndFrame();
	framegraph_interface::EndFrame();

	framegraph_interface::Submit();

	xr_interface::ReleaseImage();
	xr_interface::SubmitXRFrame();

	framegraph_interface::Present();

}
