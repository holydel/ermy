#include <ermy_commandlist.h>
#ifdef ERMY_GAPI_D3D12
#include <cassert>
#include "d3d12_rendering.h"

using namespace ermy;
using namespace ermy::rendering;


void CommandList::SetPSO(PSOID pso)
{
	ID3D12GraphicsCommandList* cbuff = static_cast<ID3D12GraphicsCommandList*>(nativeHandle);
	cbuff->SetPipelineState(gAllPSOs[pso.handle]);
	cbuff->SetGraphicsRootSignature(gAllSignatures[pso.handle]);
	cbuff->IASetPrimitiveTopology(gAllPSOMetas[pso.handle].topology);
}

void CommandList::Draw(int numVertices, int numInstanced)
{
	ID3D12GraphicsCommandList* cbuff = static_cast<ID3D12GraphicsCommandList*>(nativeHandle);
	cbuff->DrawInstanced(numVertices, numInstanced, 0, 0);
}

void CommandList::DrawIndexed(int numIndices, int numInstanced)
{
	ID3D12GraphicsCommandList* cbuff = static_cast<ID3D12GraphicsCommandList*>(nativeHandle);
	cbuff->DrawIndexedInstanced(numIndices, numInstanced, 0, 0, 0);
}

void CommandList::SetViewport(int x, int y, int width, int height)
{
	ID3D12GraphicsCommandList* cbuff = static_cast<ID3D12GraphicsCommandList*>(nativeHandle);
	D3D12_VIEWPORT viewport{x,y + height,width,-height,0,1};
	cbuff->RSSetViewports(1, &viewport);
}

void CommandList::SetScissor(int x, int y, int width, int height)
{
	ID3D12GraphicsCommandList* cbuff = static_cast<ID3D12GraphicsCommandList*>(nativeHandle);
	D3D12_RECT sissor{ x,y,x + width ,y + height };
	cbuff->RSSetScissorRects(1, &sissor);
}

void CommandList::SetRootConstants(void* data, int size)
{
	ID3D12GraphicsCommandList* cbuff = static_cast<ID3D12GraphicsCommandList*>(nativeHandle);
	cbuff->SetGraphicsRoot32BitConstants(0, size / 4, data, 0);
	//VkCommandBuffer cbuff = static_cast<VkCommandBuffer>(nativeHandle);
	//vkCmdPushConstants(cbuff, gAllPipelineLayouts[gCurrentPSOID.handle], VK_SHADER_STAGE_VERTEX_BIT, 0, size, data);
}
#endif