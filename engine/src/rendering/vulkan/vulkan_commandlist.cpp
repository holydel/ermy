#include <ermy_commandlist.h>
#ifdef ERMY_GAPI_VULKAN
#include <cassert>
#include "vulkan_rendering.h"
#include "vk_utils.h"

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

void CommandList::SetRootConstants(const void* data, int size, ermy::ShaderStage stage, int offset)
{
	VkCommandBuffer cbuff = static_cast<VkCommandBuffer>(nativeHandle);
	vkCmdPushConstants(cbuff, gAllPipelineLayouts[gCurrentPSOID.handle], vk_utils::VkShaderStageFromErmy(stage), offset, size, data);
}

void CommandList::SetDescriptorSet(int set, u64 handle)
{
	VkCommandBuffer cbuff = static_cast<VkCommandBuffer>(nativeHandle);
	auto ds = reinterpret_cast<const VkDescriptorSet>(handle);
	vkCmdBindDescriptorSets(cbuff, VK_PIPELINE_BIND_POINT_GRAPHICS, gAllPipelineLayouts[gCurrentPSOID.handle], set, 1, &ds, 0, nullptr);
}

void CommandList::InsertDebugMark(const char* u8mark)
{
	if (!vkCmdInsertDebugUtilsLabelEXT)
		return;

	VkCommandBuffer cbuff = static_cast<VkCommandBuffer>(nativeHandle);
	VkDebugUtilsLabelEXT info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
	info.pLabelName = u8mark;
	info.color[0] = 1.0f;
	info.color[1] = 1.0f;
	info.color[2] = 1.0f;
	info.color[3] = 1.0f;
	vkCmdInsertDebugUtilsLabelEXT(cbuff, &info);
}

void CommandList::BeginDebugScope(const char* u8mark)
{
	if (!vkCmdBeginDebugUtilsLabelEXT)
		return;

	VkCommandBuffer cbuff = static_cast<VkCommandBuffer>(nativeHandle);
	VkDebugUtilsLabelEXT info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT };
	info.pLabelName = u8mark;
	info.color[0] = 1.0f;
	info.color[1] = 1.0f;
	info.color[2] = 1.0f;
	info.color[3] = 1.0f;
	vkCmdBeginDebugUtilsLabelEXT(cbuff, &info);
}

void CommandList::EndDebugScope()
{
	if (!vkCmdEndDebugUtilsLabelEXT)
		return;

	VkCommandBuffer cbuff = static_cast<VkCommandBuffer>(nativeHandle);
	vkCmdEndDebugUtilsLabelEXT(cbuff);	
}

static RenderpassInfo currentRPass;

void CommandList::BeginRenderPass(RenderPassID rtt, glm::vec4 clearColor)
{
	//TODO: second pass for dynamic rendering

	VkCommandBuffer cbuff = static_cast<VkCommandBuffer>(nativeHandle);
	currentRPass = gAllRenderPasses[rtt.handle];
	
	VkClearValue clearValue;
	clearValue.color.float32[0] = clearColor.x;
	clearValue.color.float32[1] = clearColor.y;
	clearValue.color.float32[2] = clearColor.z;
	clearValue.color.float32[3] = clearColor.w;

	VkRenderPassBeginInfo info = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	info.framebuffer = currentRPass.framebuffer;
	info.renderPass = currentRPass.renderpass;

	info.renderArea.extent.width = currentRPass.defaultWidth;
	info.renderArea.extent.height = currentRPass.defaultHeight;

	info.clearValueCount = 1;
	info.pClearValues = &clearValue;
	vkCmdBeginRenderPass(cbuff, &info, VK_SUBPASS_CONTENTS_INLINE);

	SetViewport(0, 0, currentRPass.defaultWidth, currentRPass.defaultHeight);
	SetScissor(0, 0, currentRPass.defaultWidth, currentRPass.defaultHeight);
}

void CommandList::EndRenderPass()
{
	//TODO: second pass for dynamic rendering

	VkCommandBuffer cbuff = static_cast<VkCommandBuffer>(nativeHandle);
	vkCmdEndRenderPass(cbuff);

	vk_utils::ImageTransition(cbuff, currentRPass.targetImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

#endif