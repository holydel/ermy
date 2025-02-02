#include <ermy_rendering.h>
#ifdef ERMY_GAPI_D3D12
#include "d3d12_rendering.h"
#include "d3d12_utils.h"

using namespace ermy;
using namespace ermy::rendering;

std::vector<ID3D12RootSignature*> gAllSignatures;
std::vector<ID3D12PipelineState*> gAllPSOs;

std::vector<ID3D12Resource*> gAllBuffers;
std::vector<PSOMeta> gAllPSOMetas;


PSOID ermy::rendering::CreatePSO(const PSODesc& desc)
{
	D3D12_ROOT_PARAMETER rootParameters[32] = {};
	int numRootParameters = 0;

	auto IAstate = CD3DX12_INPUT_LAYOUT_SUBOBJECT();
	u16 vertexStride = 0;

	IAstate.Finalize();

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(numRootParameters, rootParameters, 0, nullptr, rootSignatureFlags);

	ID3DBlob* signature = nullptr;
	ID3DBlob* error = nullptr;
	D3D_CALL(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));

	u32 curSignatureID = gAllSignatures.size();
	gAllSignatures.push_back(nullptr);
	gAllPSOs.push_back(nullptr);
	auto& meta = gAllPSOMetas.emplace_back();

	D3D_CALL(gD3DDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&gAllSignatures[curSignatureID])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = IAstate;

	psoDesc.pRootSignature = gAllSignatures[curSignatureID];
	psoDesc.VS.pShaderBytecode = desc.shaders[0].data;
	psoDesc.VS.BytecodeLength = desc.shaders[0].size;
	psoDesc.PS.pShaderBytecode = desc.shaders[1].data;
	psoDesc.PS.BytecodeLength = desc.shaders[1].size;

	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.RasterizerState.FrontCounterClockwise = true;

	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	psoDesc.DepthStencilState.DepthEnable = false;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	//psoDesc.DepthStencilState.DepthEnable = FALSE;
	//psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	//psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;
	D3D_CALL(gD3DDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&gAllPSOs[curSignatureID])));

	meta.topology = D3D12_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;//d3d12_utils::GetD3D12TopologyFromErmy(desc.topology);
	meta.stride = vertexStride;

	return PSOID{ (u32)(gAllPSOs.size() - 1)};
}

#endif