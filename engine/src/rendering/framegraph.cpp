#include "framegraph.h"
#include "framegraph_interface.h"
#include "imgui/imgui_interface.h"
#include "application.h"
#include "swapchain_interface.h"

void framegraph::Initialize(ermy::u8 numFrames)
{
	framegraph_interface::Initialize(numFrames);

	imgui_interface::Initialize();
}

void framegraph::Shutdown()
{
	framegraph_interface::Shutdown();
}

void framegraph::Process()
{
	auto& app = GetApplication();

	void* finalCmdList = framegraph_interface::BeginFrame();

	ermy::rendering::CommandList clist(finalCmdList);
	

	framegraph_interface::BeginFinalRenderPass();

	int finalPassWidth = swapchain::GetWidth();
	int finalPassHeight = swapchain::GetHeight();
	clist.SetViewport(0, 0, finalPassWidth, finalPassHeight);
	clist.SetScissor(0, 0, finalPassWidth, finalPassHeight);

	imgui_interface::BeginFrame(finalCmdList);

	app.OnBeginFrame(clist);
	app.OnIMGUI();
	imgui_interface::EndFrame(finalCmdList);

	framegraph_interface::EndFinalRenderPass();

	app.OnEndFrame();
	framegraph_interface::EndFrame();

	framegraph_interface::Submit();

	framegraph_interface::Present();

}
