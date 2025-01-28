#include "framegraph.h"
#include "framegraph_interface.h"
#include "imgui/imgui_interface.h"

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
	framegraph_interface::BeginFrame();

	void* finalCmdList = framegraph_interface::BeginFinalRenderPass();


	imgui_interface::NewFrame(finalCmdList);
	framegraph_interface::EndFinalRenderPass();

	framegraph_interface::EndFrame();

	framegraph_interface::Submit();

	framegraph_interface::Present();

}
