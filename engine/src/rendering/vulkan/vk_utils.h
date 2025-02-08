#pragma once
#include "ermy_vulkan.h"
#include "ermy_shader.h"
#include <ermy_rendering.h>

#ifdef ERMY_GAPI_VULKAN

namespace vk_utils
{
	VkShaderStageFlagBits VkShaderStageFromErmy(ermy::ShaderStage stage);
	VkPrimitiveTopology VkPrimitiveTopologyFromErmy(ermy::rendering::PrimitiveTopology topology);

	void ImageTransition(VkCommandBuffer cbuff, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
}


#endif