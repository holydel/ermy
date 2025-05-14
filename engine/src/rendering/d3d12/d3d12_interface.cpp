#include "d3d12_interface.h"
#include "../../application.h"
#include "../../os/os.h"
#include <ermy_log.h>


#ifdef ERMY_GAPI_D3D12

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

IDXGIFactory4* gD3DFactory = nullptr;
ID3D12Debug1* gDebugController = nullptr;
IDXGIAdapter1* gD3DAdapter = nullptr;
ID3D12Device* gD3DDevice = nullptr;
ID3D12DebugDevice* gD3DDebugDevice = nullptr;
ID3D12CommandQueue* gD3DCommandQueue = nullptr;
ID3D12CommandAllocator* gD3DCommandAllocator = nullptr;
ID3D12DescriptorHeap* gDescriptorsHeapRTV = nullptr;
ID3D12DescriptorHeap* gDescriptorsHeapDSV = nullptr;

D3D12MA::Allocator* gAllocator = nullptr;
DXGI_FORMAT gD3DSwapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;


using namespace ermy;

i8 ChoosePhysicalDeviceByHeuristics(std::vector<DXGI_ADAPTER_DESC1>& adescs)
{
	return 0;
}

void ChoosePhysicalDevice()
{
	const auto& renderCfg = GetApplication().staticConfig.render;

	IDXGIAdapter1* adapter = nullptr;
	std::vector<DXGI_ADAPTER_DESC1> allAdapterDescs;
	allAdapterDescs.reserve(8);

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != gD3DFactory->EnumAdapters1(i, &adapter); ++i)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);
		allAdapterDescs.push_back(desc);

		char descStrUtf8[256] = {};
		os::WCSToUTF8(desc.Description, descStrUtf8, 256);
		ERMY_LOG("Found D3D12 device (%d): %s", i, descStrUtf8);
		adapter->Release();
	}

	auto selectedAdapterID = renderCfg.adapterID == -1 ? ChoosePhysicalDeviceByHeuristics(allAdapterDescs) : renderCfg.adapterID;

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != gD3DFactory->EnumAdapters1(i, &adapter); ++i)
	{
		if (i == selectedAdapterID)
		{
			gD3DAdapter = adapter;
			auto& desc = allAdapterDescs[i];

			char descStrUtf8[256] = {};
			os::WCSToUTF8(desc.Description, descStrUtf8, 256);
			ERMY_LOG("Selected D3D12 device (%d): %s", i, descStrUtf8);
			break;
		}
		else
		{
			adapter->Release();
		}
	}
}

const char* rendering_interface::GetName()
{
	return "D3D12";
}

void rendering_interface::Initialize()
{
	ERMY_LOG("LowLevel Rendering Interface Initialize: D3D12");

	const auto& renderCfg = GetApplication().staticConfig.render;

	UINT dxgiFactoryFlags = 0;

	if (renderCfg.enableDebugLayers)
	{
		ID3D12Debug* dc;
		D3D_CALL(D3D12GetDebugInterface(IID_PPV_ARGS(&dc)));
		D3D_CALL(dc->QueryInterface(IID_PPV_ARGS(&gDebugController)));
		gDebugController->EnableDebugLayer();
		gDebugController->SetEnableGPUBasedValidation(true);

		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

		dc->Release();
		dc = nullptr;
	}

	D3D_CALL(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&gD3DFactory)));

	ChoosePhysicalDevice();

	D3D_CALL(D3D12CreateDevice(gD3DAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&gD3DDevice)));

	if (renderCfg.enableDebugLayers)
	{
		D3D_CALL(gD3DDevice->QueryInterface(&gD3DDebugDevice));
	}


	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	D3D_CALL(gD3DDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&gD3DCommandQueue)));

	D3D_CALL(gD3DDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&gD3DCommandAllocator)));


	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = 64;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		D3D_CALL(gD3DDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&gDescriptorsHeapRTV)));
		gDescriptorsHeapRTV->SetName(L"RTV Heap");
	}

	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = 64;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		D3D_CALL(gD3DDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&gDescriptorsHeapDSV)));
		gDescriptorsHeapDSV->SetName(L"DSV Heap");
	}

	D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
	allocatorDesc.pDevice = gD3DDevice;
	allocatorDesc.pAdapter = gD3DAdapter;
	// These flags are optional but recommended.
	allocatorDesc.Flags = D3D12MA::ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED |
		D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED;

	HRESULT hr = D3D12MA::CreateAllocator(&allocatorDesc, &gAllocator);
}

void rendering_interface::Shutdown()
{
	ERMY_LOG("LowLevel Rendering Interface Shutdown: D3D12");
}

void rendering_interface::Process()
{
	
}
#endif

