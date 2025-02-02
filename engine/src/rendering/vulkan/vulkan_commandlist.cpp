#include <ermy_commandlist.h>
#ifdef ERMY_GAPI_VULKAN
#include <cassert>
#include "vulkan_rendering.h"

using namespace ermy;
using namespace ermy::rendering;


void CommandList::SetPSO(PSOID pso)
{
	VkCommandBuffer cbuff = static_cast<VkCommandBuffer>(nativeHandle);
	assert(pso.handle >= 0 && pso.handle < gAllPipelines.size());

	vkCmdBindPipeline(cbuff, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, gAllPipelines[pso.handle]);
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

	VkViewport viewport = {x,y,width,height,0,1};
	vkCmdSetViewport(cbuff, 0, 1, &viewport);
	
}

void CommandList::SetScissor(int x, int y, int width, int height)
{
	VkCommandBuffer cbuff = static_cast<VkCommandBuffer>(nativeHandle);
	VkRect2D scissor = { {x,y},{width,height} };
	vkCmdSetScissor(cbuff, 0, 1, &scissor);
}

#endif