#include "vk_utils.h"

#ifdef ERMY_GAPI_VULKAN

VkShaderStageFlagBits vk_utils::VkShaderStageFromErmy(ermy::ShaderStage stage)
{
	switch (stage)
	{
	case ermy::ShaderStage::Vertex:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
	case ermy::ShaderStage::Fragment:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
	case ermy::ShaderStage::Compute:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;
	}

	return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
}
#endif