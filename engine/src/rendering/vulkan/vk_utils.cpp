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
#endif