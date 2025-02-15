#include <ermy_commandlist.h>
#ifdef ERMY_GAPI_VULKAN
#include <cassert>
#include "vulkan_rendering.h"

using namespace ermy;
using namespace ermy::rendering;


PSOID gCurrentPSOID;

void CommandList::SetPSO(PSOID pso)
{
	VkCommandBuffer cbuff = static_cast<VkCommandBuffer>(nativeHandle);
	assert(pso.handle >= 0 && pso.handle < gAllPipelines.size());

	vkCmdBindPipeline(cbuff, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, gAllPipelines[pso.handle]);
	gCurrentPSOID = pso;
}

void CommandList::Draw(int numVertices, int numInstances)
{
	VkCommandBuffer cbuff = static_cast<VkCommandBuffer>(nativeHandle);

	vkCmdDraw(cbuff, numVertices, numInstances, 0, 0);
}

void CommandList::DrawIndexed(int numIndices, int numInstances)
{
	VkCommandBuffer cbuff = static_cast<VkCommandBuffer>(nativeHandle);

	vkCmdDrawIndexed(cbuff, numIndices, numInstances, 0, 0, 0);
}

void CommandList::SetViewport(int x, int y, int width, int height)
{
	VkCommandBuffer cbuff = static_cast<VkCommandBuffer>(nativeHandle);

	VkViewport viewport = {static_cast<float>(x),static_cast<float>(y)
		,static_cast<float>(width),static_cast<float>(height),0,1};
	vkCmdSetViewport(cbuff, 0, 1, &viewport);
	
}

void CommandList::SetScissor(int x, int y, int width, int height)
{
	VkCommandBuffer cbuff = static_cast<VkCommandBuffer>(nativeHandle);
	VkRect2D scissor = { {x,y},{static_cast<u32>(width),static_cast<u32>(height)} };
	vkCmdSetScissor(cbuff, 0, 1, &scissor);
}

void CommandList::SetRootConstants(void* data, int size)
{
	VkCommandBuffer cbuff = static_cast<VkCommandBuffer>(nativeHandle);
	vkCmdPushConstants(cbuff, gAllPipelineLayouts[gCurrentPSOID.handle], VK_SHADER_STAGE_VERTEX_BIT, 0, size, data);
}

void CommandList::InsertDebugMark(const char* u8mark)
{
	VkCommandBuffer cbuff = static_cast<VkCommandBuffer>(nativeHandle);
	VkDebugMarkerMarkerInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT };
	info.pMarkerName = u8mark;
	info.color[0] = 1.0f;
	info.color[1] = 1.0f;
	info.color[2] = 1.0f;
	info.color[3] = 1.0f;
	vkCmdDebugMarkerInsertEXT(cbuff, &info);
}

void CommandList::BeginDebugScope(const char* u8mark)
{
	VkCommandBuffer cbuff = static_cast<VkCommandBuffer>(nativeHandle);
	VkDebugMarkerMarkerInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT };
	info.pMarkerName = u8mark;
	info.color[0] = 1.0f;
	info.color[1] = 1.0f;
	info.color[2] = 1.0f;
	info.color[3] = 1.0f;
	vkCmdDebugMarkerBeginEXT(cbuff, &info);
}

void CommandList::EndDebugScope()
{
	VkCommandBuffer cbuff = static_cast<VkCommandBuffer>(nativeHandle);
	vkCmdDebugMarkerEndEXT(cbuff);
	
}

#endif