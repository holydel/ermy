#include "framegraph.h"
#include "framegraph_interface.h"
#include "imgui/imgui_interface.h"
#include "application.h"
#include "swapchain_interface.h"
#include "graphics/canvas_interface.h"
#include "rendering.h"
#include "scene/scene.h"

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

	app.OnUpdate();

	void* finalCmdList = framegraph_interface::BeginFrame();
	ermy::rendering::CommandList clist(finalCmdList);
	
	int finalPassWidth = swapchain::GetWidth();
	int finalPassHeight = swapchain::GetHeight();

	gErmyFrameConstants.canvasRepSizeHalf = glm::vec2(2.0 / float(finalPassWidth), 2.0 / float(finalPassHeight));
	gErmyFrameConstants.canvasClearColor.r = canvas_interface::BgColor[0];
	gErmyFrameConstants.canvasClearColor.g = canvas_interface::BgColor[1];
	gErmyFrameConstants.canvasClearColor.b = canvas_interface::BgColor[2];
	gErmyFrameConstants.canvasClearColor.a = canvas_interface::BgColor[3];

	gErmyFrameConstants.CameraViewDir[0].x = 2.0f;
	gErmyFrameConstants.CameraViewDir[1].x = 3.0f;

	gErmyFrameConstants.CameraWorldPos[0].x = 4.0f;
	gErmyFrameConstants.CameraWorldPos[1].x = 5.0f;

	gErmyFrameConstants.ViewProjMatrixInv[0][0].x = 6.0f;
	gErmyFrameConstants.ViewProjMatrixInv[1][0].x = 7.0f;
	gErmyFrameConstants.canvasPreRotate[0].x = 8.0f;
	gErmyFrameConstants.canvasPreRotate[1].y = 9.0f;

	scene_internal::UpdateUniforms();
	UpdateBufferData(gFrameConstants, &gErmyFrameConstants);
	app.OnBeginFrame(clist);

	framegraph_interface::BeginFinalRenderPass();

	clist.SetViewport(0, 0, finalPassWidth, finalPassHeight);
	clist.SetScissor(0, 0, finalPassWidth, finalPassHeight);

	imgui_interface::BeginFrame(finalCmdList);
	//render scene
	scene_internal::Render(clist);
	//render canvas
	canvas_interface::SetCommandList(&clist);

	app.OnBeginFinalPass(clist);
	app.OnIMGUI();
	imgui_interface::EndFrame(finalCmdList);

	framegraph_interface::EndFinalRenderPass();

	app.OnEndFrame();
	framegraph_interface::EndFrame();

	framegraph_interface::Submit();

	framegraph_interface::Present();

}
