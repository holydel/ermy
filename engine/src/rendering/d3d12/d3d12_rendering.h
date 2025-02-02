#include <ermy_rendering.h>
#ifdef ERMY_GAPI_D3D12
#include "d3d12_interface.h"
#include <vector>

extern std::vector<CD3DX12_SHADER_BYTECODE> gAllShaders;
extern std::vector<ID3D12RootSignature*> gAllSignatures;
extern std::vector<ID3D12PipelineState*> gAllPSOs;

extern D3D12MA::Allocator* gAllocator;

extern std::vector<ID3D12Resource*> gAllBuffers;

struct BufferMeta
{
	void* mappedPointer;
	D3D12MA::Allocation* allocation;
	D3D12_GPU_VIRTUAL_ADDRESS bufferGPULocation;
	UINT sizeInBytes;
	//VmaAllocation allocation;
	//VmaAllocationInfo allocInfo;
	//VkDeviceMemory dedicatedMemory;
	//VkDeviceAddress deviceAddress;
};

extern std::vector<BufferMeta> gAllBuffersMeta;

struct PSOMeta
{
	D3D12_PRIMITIVE_TOPOLOGY topology;
	ermy::u16 stride;
};

extern std::vector<PSOMeta> gAllPSOMetas;
#endif