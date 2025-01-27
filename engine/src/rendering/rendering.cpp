#include "rendering.h"
#include "rendering_interface.h"
#include "swapchain_interface.h"
#include "framegraph.h"

namespace rendering
{
	void Initialize()
	{
		rendering_interface::Initialize();
		swapchain::Initialize();
		framegraph::Initialize(swapchain::GetNumFrames());
	}

	void Shutdown()
	{
		framegraph::Shutdown();
		swapchain::Shutdown();
		rendering_interface::Shutdown();
	}

	void Process()
	{
		//TODO: build framegraph	
		//TODO: execute framegraph
		framegraph::Process();
		//swapchain::Process();
	}

	void BeginFrame()
	{
		//swapchain::AcquireNextImage();
	}

	void EndFrame()
	{
		//swapchain::Present();
	}
}