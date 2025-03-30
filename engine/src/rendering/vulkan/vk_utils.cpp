#include "vk_utils.h"

#ifdef ERMY_GAPI_VULKAN
#include "vulkan_interface.h"

using namespace ermy;
using namespace ermy::rendering;

VkFormat vk_utils::VkFormatFromErmy(ermy::rendering::Format format)
{
	switch (format)
	{
	case Format::RGBA8_UNORM:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case Format::ARGB8_UNORM:
		return VK_FORMAT_A8B8G8R8_UNORM_PACK32;
	case Format::RGBA16_UNORM:
		return VK_FORMAT_R16G16B16A16_UNORM;
	case Format::RGBA16_NORM:
		return VK_FORMAT_R16G16B16A16_SNORM;
	case  Format::RGBA16F:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case  Format::RG16F:
		return VK_FORMAT_R16G16_SFLOAT;
	case  Format::R16F:
		return VK_FORMAT_R16_SFLOAT;
	case  Format::RGBA16_UINT:
		return VK_FORMAT_R16G16B16A16_UINT;
	case  Format::RG16_UINT:
		return VK_FORMAT_R16G16_UINT;
	case  Format::R16_UINT:
		return VK_FORMAT_R16_UINT;
	case  Format::RG16_UNORM:
		return VK_FORMAT_R16G16_UNORM;
	case  Format::R16_UNORM:
		return VK_FORMAT_R16_UNORM;
	case  Format::RGBA8_UINT:
		return VK_FORMAT_R8G8B8A8_UINT;
	case  Format::R32F:
		return VK_FORMAT_R32_SFLOAT;
	case  Format::RG32F:
		return VK_FORMAT_R32G32_SFLOAT;
	case  Format::RGB32F:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case  Format::RGBA32F:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case  Format::BC1:
		return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
	case  Format::BC2:
		return VK_FORMAT_BC2_SRGB_BLOCK;
	case  Format::BC3:
		return VK_FORMAT_BC3_SRGB_BLOCK;
	case  Format::BC4:
		return VK_FORMAT_BC4_UNORM_BLOCK;
	case  Format::BC5:
		return VK_FORMAT_BC5_UNORM_BLOCK;
	case  Format::BC6:
		return VK_FORMAT_BC6H_UFLOAT_BLOCK;
	case  Format::BC6_SF:
		return VK_FORMAT_BC6H_SFLOAT_BLOCK;
	case  Format::BC7:
		return VK_FORMAT_BC7_SRGB_BLOCK;
	case  Format::R8_UNORM:
		return VK_FORMAT_R8_UNORM;
	case  Format::RG8_UNORM:
		return VK_FORMAT_R8G8_UNORM;
	case Format::BGRA8_UNORM:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case Format::D32F:
		return VK_FORMAT_D32_SFLOAT;
	case Format::D16_UNORM:
		return VK_FORMAT_D16_UNORM;
	case Format::D24_UNORM_S8_UINT:
		return VK_FORMAT_D24_UNORM_S8_UINT;
	case Format::RGBA8_SRGB:
		return VK_FORMAT_R8G8B8A8_SRGB;
	}

	return VK_FORMAT_R8G8B8A8_SRGB;
}

VkBufferUsageFlags  vk_utils::VkBufferUsageFromErmy(ermy::rendering::BufferUsage busage)
{
	switch (busage) {
	case ermy::rendering::BufferUsage::Vertex:
		return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	case ermy::rendering::BufferUsage::Index:
		return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	case ermy::rendering::BufferUsage::Uniform:
		return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	case ermy::rendering::BufferUsage::Storage:
		return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	case ermy::rendering::BufferUsage::TransferDst:
		return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	case ermy::rendering::BufferUsage::TransferSrc:
		return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	default:
		// Handle unknown or unsupported buffer usage
		return VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	}
}
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
	case ShaderStage::Geometry:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT;
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

	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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

void vk_utils::ImageTransition(VkCommandBuffer cbuff, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectMask, int numMips, int numLayers)
{
	const VkImageMemoryBarrier2 barrier = createImageMemoryBarrier(image, oldLayout, newLayout, { aspectMask, 0, (uint32_t)numMips, 0, (uint32_t)numLayers });
	const VkDependencyInfo depInfo{ .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &barrier };

	vkCmdPipelineBarrier2(cbuff, &depInfo);
}

void vk_utils::BufferMemoryBarrier(VkCommandBuffer cbuff, VkBuffer buffer, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage)
{
	VkBufferMemoryBarrier bufferBarrier = {};
	bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	bufferBarrier.srcAccessMask = srcAccess;
	bufferBarrier.dstAccessMask = dstAccess;
	bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	bufferBarrier.buffer = buffer;
	bufferBarrier.size = VK_WHOLE_SIZE;
	vkCmdPipelineBarrier(cbuff, srcStage, dstStage, 0, 0, nullptr, 1, &bufferBarrier, 0, nullptr);
}

VkBufferImageCopy vk_utils::MakeBufferImageCopy(VkExtent3D extent, VkImageAspectFlags aspectMask, int numMips, int numLayers)
{
	VkBufferImageCopy copyRegion = {};
	copyRegion.bufferOffset = 0;
	copyRegion.bufferRowLength = 0;
	copyRegion.bufferImageHeight = 0;
	copyRegion.imageSubresource.aspectMask = aspectMask;
	copyRegion.imageSubresource.mipLevel = 0;
	copyRegion.imageSubresource.baseArrayLayer = 0;
	copyRegion.imageSubresource.layerCount = 1;
	copyRegion.imageOffset = { 0, 0, 0 };
	copyRegion.imageExtent = extent;
	return copyRegion;
}

#endif

void vk_utils::debug::_setObjectName(u64 objHandle, VkObjectType objType, const char* name)
{
	if (vkSetDebugUtilsObjectNameEXT)
	{
		VkDebugUtilsObjectNameInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
		info.objectHandle = objHandle;
		info.objectType = objType;
		info.pObjectName = name;

		VK_CALL(vkSetDebugUtilsObjectNameEXT(gVKDevice, &info));
	}
}
