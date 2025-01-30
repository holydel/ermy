#pragma once
#include "d3d12_interface.h"

#ifdef ERMY_GAPI_D3D12
#include "../swapchain_interface.h"
extern IDXGISwapChain3* gSwapChain;
extern DXGI_FORMAT gSwapChainFormat;

extern ID3D12Resource* gIntermediateMSAATarget;
extern ID3D12Resource* gIntermediateDSBuffer; //can be multisampled or not

struct BackbufferResourceInfo
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE bbRTV;
	ID3D12Resource* bbResource;
	int frameIndex;
};

BackbufferResourceInfo& GetCurrentBackbufferResourceInfo();
#endif