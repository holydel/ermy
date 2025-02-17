#include "rendering.h"
#include "rendering_interface.h"
#include "swapchain_interface.h"
#include "framegraph.h"
#include "ermy_rendering.h"

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

using namespace ermy::rendering;

FormatInfo ermy::rendering::GetFormatInfo(Format format)
{
	switch (format)
	{
	case Format::RGBA8_UNORM:
		return FormatInfo{ 4 };
	case Format::RGBA16_UNORM:
		return FormatInfo{ 8 };
	case Format::RGBA16_NORM:
		return FormatInfo{ 8 };
	case  Format::RGBA16F:
		return FormatInfo{ 8 };
	case  Format::RGBA16_UINT:
		return FormatInfo{ 8 };
	case  Format::RG16_UINT:
		return FormatInfo{ 4 };
	case  Format::R16_UINT:
		return FormatInfo{ 2 };
	case  Format::RG16_UNORM:
		return FormatInfo{ 4 };
	case  Format::R16_UNORM:
		return FormatInfo{ 2 };
	case  Format::RGBA8_UINT:
		return FormatInfo{ 4 };
	}

	return FormatInfo{ 4 };
}