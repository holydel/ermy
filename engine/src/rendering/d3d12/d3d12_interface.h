#pragma once
#include "../rendering_interface.h"
#ifdef ERMY_GAPI_D3D12
#define RPC_NO_WINDOWS_H

#include <directx/d3d12.h>
#include "directx/d3dx12.h"
#include <dxgi1_4.h>
#include <D3D12MemAlloc.h>
#include <system_error>
#include "../../os/os.h"

extern IDXGIFactory4* gD3DFactory;
extern ID3D12Debug1* gDebugController;
extern IDXGIAdapter1* gD3DAdapter;
extern ID3D12Device* gD3DDevice;
extern ID3D12DebugDevice* gD3DDebugDevice;
extern ID3D12CommandQueue* gD3DCommandQueue;
extern ID3D12CommandAllocator* gD3DCommandAllocator;
extern ID3D12DescriptorHeap* gDescriptorsHeapRTV;
extern ID3D12DescriptorHeap* gDescriptorsHeapDSV;
extern ID3D12GraphicsCommandList* gCurrentCommandBuffer;

extern std::vector<CD3DX12_SHADER_BYTECODE> gAllShaders;
extern std::vector<ID3D12RootSignature*> gAllSignatures;
extern std::vector<ID3D12PipelineState*> gAllPSOs;

extern D3D12MA::Allocator* gAllocator;
extern DXGI_FORMAT gD3DSwapChainFormat;
#define D3D_CALL(func) {HRESULT res = (func); if(res < 0){ os::FatalFail( std::system_category().message(res).c_str() ); } }
#endif