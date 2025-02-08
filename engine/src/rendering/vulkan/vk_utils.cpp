#include "vk_utils.h"

#ifdef ERMY_GAPI_VULKAN
using namespace ermy;
using namespace ermy::rendering;

VkShaderStageFlagBits vk_utils::VkShaderStageFromErmy(ermy::ShaderStage stage)
{
	switch (stage)
	{
	case ShaderStage::Vertex:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
	case ShaderStage::Fragment:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
	case ShaderStage::Compute:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;
	}

	return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
}

VkPrimitiveTopology vk_utils::VkPrimitiveTopologyFromErmy(PrimitiveTopology topology)
{
	switch (topology)
	{
	case PrimitiveTopology::TriangleList:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	case PrimitiveTopology::TriangleStrip:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	case PrimitiveTopology::LineList:
		return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	case PrimitiveTopology::LineStrip:
		return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
	case PrimitiveTopology::Points:
		return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	}
}

static std::tuple<VkPipelineStageFlags2, VkAccessFlags2> makePipelineStageAccessTuple(VkImageLayout state)
{
	switch (state)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		return std::make_tuple(VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE);
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		return std::make_tuple(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		return std::make_tuple(VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT
			| VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT,
			VK_ACCESS_2_SHADER_READ_BIT);
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		return std::make_tuple(VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT);
	case VK_IMAGE_LAYOUT_GENERAL:
		return std::make_tuple(VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_TRANSFER_BIT,
			VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT);
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		return std::make_tuple(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_NONE);
	default: {
		return std::make_tuple(VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT);
	}
	}
};

static VkImageMemoryBarrier2 createImageMemoryBarrier(VkImage       image,
	VkImageLayout oldLayout,
	VkImageLayout newLayout,
	VkImageSubresourceRange subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT,
												0, 1, 0, 1 })
{
	const auto [srcStage, srcAccess] = makePipelineStageAccessTuple(oldLayout);
	const auto [dstStage, dstAccess] = makePipelineStageAccessTuple(newLayout);

	VkImageMemoryBarrier2 barrier{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
								  .srcStageMask = srcStage,
								  .srcAccessMask = srcAccess,
								  .dstStageMask = dstStage,
								  .dstAccessMask = dstAccess,
								  .oldLayout = oldLayout,
								  .newLayout = newLayout,
								  .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
								  .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
								  .image = image,
								  .subresourceRange = subresourceRange };
	return barrier;
}

void vk_utils::ImageTransition(VkCommandBuffer cbuff, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	const VkImageMemoryBarrier2 barrier = createImageMemoryBarrier(image, oldLayout, newLayout, { aspectMask, 0, 1, 0, 1 });
	const VkDependencyInfo depInfo{ .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &barrier };

	vkCmdPipelineBarrier2(cbuff, &depInfo);
}
#endif