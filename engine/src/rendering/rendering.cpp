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
		auto& config = ermy::Application::GetApplication()->staticConfig;

		if (config.HasOutputWindow())
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
	//u8 blockSize;
	//u8 channelsCount;
	//u8 blockWidth;
	//u8 blockHeight;
	//u8 blockDepth
	//u8 isSRGB
	switch (format)
	{
	case Format::R8_UNORM:
		return FormatInfo{ 1,1,1,1,1,0 };
	case Format::RG8_UNORM:
		return FormatInfo{ 2,1,1,1,1,0 };
	case Format::RGBA8_UNORM:
		return FormatInfo{ 4,4,1,1,1,0 };
	case Format::RGBA8_SRGB:
		return FormatInfo{ 4, 1, 1, 1,1,1 };
	case Format::ARGB8_UNORM:
		return FormatInfo{ 4,4,1,1,1,0 };
	case Format::RGBA16_UNORM:
		return FormatInfo{ 8,4,1,1,1,0 };
	case Format::RGBA16_NORM:
		return FormatInfo{ 8,4,1,1,1,0 };
	case  Format::RGBA16F:
		return FormatInfo{ 8,2,1,1,1,0 };
	case  Format::RG16F:
		return FormatInfo{ 4,1,1,1,1,0 };
	case  Format::R16F:
		return FormatInfo{ 2,4,1,1,1,0 };
	case  Format::RGBA16_UINT:
		return FormatInfo{ 8,4,1,1,1,0 };
	case  Format::RG16_UINT:
		return FormatInfo{ 4,2,1,1,1,0 };
	case  Format::R16_UINT:
		return FormatInfo{ 2,1,1,1,1,0 };
	case  Format::RG16_UNORM:
		return FormatInfo{ 4,2,1,1,1,0 };
	case  Format::R16_UNORM:
		return FormatInfo{ 2,1,1,1,1,0 };
	case  Format::RGBA8_UINT:
		return FormatInfo{ 4,4,1,1,1,0 };
	case  Format::R32F:
		return FormatInfo{ 4,1,1,1,1,0 };
	case  Format::RG32F:
		return FormatInfo{ 8,2,1,1,1,0 };
	case  Format::RGB32F:
		return FormatInfo{ 12,3,1,1,1,0 };
	case  Format::RGBA32F:
		return FormatInfo{ 16,4,1,1,1,0 };
	case Format::BGRA8_UNORM:
		return FormatInfo{ 4,4,1,1,1,0 };
	case Format::D32F:
		return FormatInfo{ 4,1,1,1,1,0 };
	case Format::D16_UNORM:
		return FormatInfo{ 2,1,1,1,1,0 };
	case Format::D24_UNORM_S8_UINT:
		return FormatInfo{ 4,2,1,1,1,0 };
		// Block-compressed formats
	case Format::BC1:
		return FormatInfo{ 8, 4, 4, 4 ,1,0 }; // BC1 (DXT1) uses 8 bytes per 4x4 block
	case Format::BC2:
		return FormatInfo{ 16, 4, 4, 4 ,1,0 }; // BC2 (DXT3) uses 16 bytes per 4x4 block
	case Format::BC3:
		return FormatInfo{ 16, 4, 4, 4 ,1,0 }; // BC3 (DXT5) uses 16 bytes per 4x4 block
	case Format::BC4:
		return FormatInfo{ 8, 1, 4, 4 ,1,0 }; // BC4 uses 8 bytes per 4x4 block (single channel)
	case Format::BC5:
		return FormatInfo{ 16, 2, 4, 4,1,0 }; // BC5 uses 16 bytes per 4x4 block (two channels)
	case Format::BC6:
		return FormatInfo{ 16, 3, 4, 4,1,0 }; // BC6 uses 16 bytes per 4x4 block (three channels, HDR)
	case Format::BC6_SF:
		return FormatInfo{ 16, 3, 4, 4,1,0 }; // BC6 uses 16 bytes per 4x4 block (three channels, HDR)
	case Format::BC7:
		return FormatInfo{ 16, 4, 4, 4,1,0 }; // BC7 uses 16 bytes per 4x4 block (four channels)
	}

	return FormatInfo{ 4,1,1,1 };
}

bool ermy::rendering::IsDepthFormat(Format format)
{
	switch (format)
	{
	case Format::D32F:
	case Format::D16_UNORM:
	case Format::D24_UNORM_S8_UINT:
		return true;
	}
	return false;
}

ErmyFrame gErmyFrameConstants;