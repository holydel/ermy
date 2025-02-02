#include "d3d12_swapchain.h"
#include <chrono>
#include "../../os/os.h"

using namespace ermy;

#ifdef ERMY_GAPI_D3D12

int gNumFrames = 3;
std::vector<BackbufferResourceInfo> gBBFrames;
IDXGISwapChain3* gSwapChain = nullptr;
u64 gCurrentBBResourceIndex = 0;
u64 gFrameID = 0;
DXGI_FORMAT gSwapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
u32 gNewWidth = 0;
u32 gCurWidth = 0;
u32 gNewHeight = 0;
u32 gCurHeight = 0;
u32 gDeltaFrameInBB = 0;

//ID3D12Resource* gIntermediateMSAATarget = nullptr;
//ID3D12Resource* gIntermediateDSBuffer = nullptr; //can be multisampled or not
//D3D12MA::Allocation* gIntermediateMSAAAllocation = nullptr;
//D3D12MA::Allocation* gIntermediateDSBufferAllocation = nullptr;

void swapchain::Initialize()
{
	HWND hwnd = static_cast<HWND>(os::GetNativeWindowHandle());

	RECT clientRect = {};
	GetClientRect(hwnd, (LPRECT)&clientRect);

	DXGI_SWAP_CHAIN_DESC1 sdesc = {};
	sdesc.Width = gNewWidth = gCurWidth = clientRect.right - clientRect.left;
	sdesc.Height = gNewHeight = gCurHeight = clientRect.bottom - clientRect.top;
	sdesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	sdesc.BufferCount = gNumFrames;
	sdesc.Format = gSwapChainFormat;
	sdesc.SampleDesc.Count = 1;
	sdesc.SampleDesc.Quality = 0;
	sdesc.Scaling = DXGI_SCALING_NONE;
	sdesc.Stereo = false;
	sdesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC sfdesc = {};
	sfdesc.Windowed = true;
	sfdesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
	sfdesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sfdesc.RefreshRate.Numerator = 1;
	sfdesc.RefreshRate.Denominator = 60;
	IDXGIOutput* output = nullptr;

	IDXGISwapChain1* swapchain1 = nullptr;
	auto res = gD3DFactory->CreateSwapChainForHwnd(gD3DCommandQueue, hwnd, &sdesc, nullptr, output, &swapchain1);
	swapchain1->QueryInterface(__uuidof(IDXGISwapChain3), (void**)&gSwapChain);

	gBBFrames.resize(gNumFrames);

	auto rtvDescriptorSize = gD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(gDescriptorsHeapRTV->GetCPUDescriptorHandleForHeapStart());

	for (uint32_t i = 0; i < gNumFrames; ++i)
	{
		BackbufferResourceInfo& bb = gBBFrames[i];

		gSwapChain->GetBuffer(i, IID_PPV_ARGS(&bb.bbResource));

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS; // Multisampled view

		gD3DDevice->CreateRenderTargetView(bb.bbResource, &rtvDesc, rtvHandle);

		bb.bbRTV = rtvHandle;
		rtvHandle.Offset(rtvDescriptorSize);
	}
}

void swapchain::AcquireNextImage()
{
	gCurrentBBResourceIndex = gSwapChain->GetCurrentBackBufferIndex();
}

BackbufferResourceInfo& GetCurrentBackbufferResourceInfo()
{
	return gBBFrames[gCurrentBBResourceIndex];
}

void swapchain::Process()
{


}

void swapchain::Present()
{
	gSwapChain->Present(0, DXGI_SWAP_EFFECT_DISCARD);
}

void swapchain::Shutdown()
{

}

int swapchain::GetNumFrames()
{
	return 3;
}

int swapchain::GetWidth()
{
	return gCurWidth;
}

int swapchain::GetHeight()
{
	return gCurHeight;
}

bool swapchain::ReInitIfNeeded()
{
	RECT clientRect = {};
	HWND hwnd = static_cast<HWND>(os::GetNativeWindowHandle());
	GetClientRect(hwnd, (LPRECT)&clientRect);

	gNewWidth = clientRect.right - clientRect.left;
	gNewHeight = clientRect.bottom - clientRect.top;

	if (gNewWidth != gCurWidth || gCurHeight != gNewHeight)
	{		
		gCurWidth = gNewWidth;
		gCurHeight = gNewHeight;

		for (uint32_t i = 0; i < gNumFrames; ++i)
		{
			gBBFrames[i].bbResource->Release();
		}

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		D3D_CALL(gSwapChain->GetDesc(&swapChainDesc));

		D3D_CALL(gSwapChain->ResizeBuffers(swapChainDesc.BufferCount, gCurWidth, gCurHeight, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));


		auto rtvDescriptorSize = gD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(gDescriptorsHeapRTV->GetCPUDescriptorHandleForHeapStart());

		for (uint32_t i = 0; i < gNumFrames; ++i)
		{
			BackbufferResourceInfo& fo = gBBFrames[i];
			gSwapChain->GetBuffer(i, IID_PPV_ARGS(&fo.bbResource));
			//gDevice->CreateRenderTargetView(fo.bbResource, nullptr, rtvHandle);

			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; // Multisampled view
			// Other RTV settings...

			// Create the RTV using textureResource and rtvDesc
			gD3DDevice->CreateRenderTargetView(fo.bbResource, &rtvDesc, rtvHandle);

			fo.bbRTV = rtvHandle;
			rtvHandle.Offset(rtvDescriptorSize);
		}
	}

	return false;
}
#endif