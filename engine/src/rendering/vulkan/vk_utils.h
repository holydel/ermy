#pragma once
#include "ermy_vulkan.h"
#include "ermy_shader.h"
#include <ermy_rendering.h>
#include <array>

#ifdef ERMY_GAPI_VULKAN

namespace vk_utils
{
	VkShaderStageFlagBits VkShaderStageFromErmy(ermy::ShaderStage stage);
	VkPrimitiveTopology VkPrimitiveTopologyFromErmy(ermy::rendering::PrimitiveTopology topology);
	VkFormat VkFormatFromErmy(ermy::rendering::Format format);
	
	void ImageTransition(VkCommandBuffer cbuff, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectMask, int numMips = 1, int numLayers = 1);
	VkBufferUsageFlagBits VkBufferUsageFromErmy(ermy::rendering::BufferUsage busage);
	namespace debug
	{
		void _setObjectName(ermy::u64 objHandle, VkObjectType objType, const char* name);

		template<typename T>
		concept VulkanObject = requires(T obj) {
			requires std::is_same_v<T, VkBuffer> ||
		std::is_same_v<T, VkCommandBuffer> ||
			std::is_same_v<T, VkCommandPool> ||
			std::is_same_v<T, VkDevice> ||
			std::is_same_v<T, VkDescriptorSetLayout> ||
			std::is_same_v<T, VkFence> ||
			std::is_same_v<T, VkFramebuffer> ||
			std::is_same_v<T, VkImage> ||
			std::is_same_v<T, VkImageView> ||
			std::is_same_v<T, VkInstance> ||
			std::is_same_v<T, VkPhysicalDevice> ||
			std::is_same_v<T, VkPipeline> ||
			std::is_same_v<T, VkPipelineLayout> ||
			std::is_same_v<T, VkRenderPass> ||
			std::is_same_v<T, VkSampler> ||
			std::is_same_v<T, VkSemaphore> ||
			std::is_same_v<T, VkShaderModule> ||
			std::is_same_v<T, VkSwapchainKHR> ||
			std::is_same_v<T, VkQueue>;
		};

		// Single template function to handle all Vulkan object types
		template<VulkanObject T>
		void SetName(T obj, const char* fmt, ...)
		{
			// Map the object type to its corresponding VkObjectType
			VkObjectType objType = []() {
				if constexpr (std::is_same_v<T, VkBuffer>)
					return VK_OBJECT_TYPE_BUFFER;
				else if constexpr (std::is_same_v<T, VkCommandBuffer>)
					return VK_OBJECT_TYPE_COMMAND_BUFFER;
				else if constexpr (std::is_same_v<T, VkCommandPool>)
					return VK_OBJECT_TYPE_COMMAND_POOL;
				else if constexpr (std::is_same_v<T, VkDevice>)
					return VK_OBJECT_TYPE_DEVICE;
				else if constexpr (std::is_same_v<T, VkDescriptorSetLayout>)
					return VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
				else if constexpr (std::is_same_v<T, VkFence>)
					return VK_OBJECT_TYPE_FENCE;
				else if constexpr (std::is_same_v<T, VkFramebuffer>)
					return VK_OBJECT_TYPE_FRAMEBUFFER;
				else if constexpr (std::is_same_v<T, VkImage>)
					return VK_OBJECT_TYPE_IMAGE;
				else if constexpr (std::is_same_v<T, VkImageView>)
					return VK_OBJECT_TYPE_IMAGE_VIEW;
				else if constexpr (std::is_same_v<T, VkInstance>)
					return VK_OBJECT_TYPE_INSTANCE;
				else if constexpr (std::is_same_v<T, VkPhysicalDevice>)
					return VK_OBJECT_TYPE_PHYSICAL_DEVICE;
				else if constexpr (std::is_same_v<T, VkPipeline>)
					return VK_OBJECT_TYPE_PIPELINE;
				else if constexpr (std::is_same_v<T, VkPipelineLayout>)
					return VK_OBJECT_TYPE_PIPELINE_LAYOUT;
				else if constexpr (std::is_same_v<T, VkRenderPass>)
					return VK_OBJECT_TYPE_RENDER_PASS;
				else if constexpr (std::is_same_v<T, VkSampler>)
					return VK_OBJECT_TYPE_SAMPLER;
				else if constexpr (std::is_same_v<T, VkSemaphore>)
					return VK_OBJECT_TYPE_SEMAPHORE;
				else if constexpr (std::is_same_v<T, VkShaderModule>)
					return VK_OBJECT_TYPE_SHADER_MODULE;
				else if constexpr (std::is_same_v<T, VkSwapchainKHR>)
					return VK_OBJECT_TYPE_SWAPCHAIN_KHR;
				else if constexpr (std::is_same_v<T, VkQueue>)
					return VK_OBJECT_TYPE_QUEUE;
				else {
#ifdef _WIN32
					static_assert(false, "Unsupported Vulkan object type");
#endif
				}
			}();

			// Buffer to hold the formatted string
			std::array<char, 256> buffer{}; // Adjust size as needed

			// Format the string
			va_list args;
			va_start(args, fmt);
			vsnprintf(buffer.data(), buffer.size(), fmt, args);
			va_end(args);
			// Call the internal helper function
			_setObjectName(reinterpret_cast<ermy::u64>(obj), objType, buffer.data());
		}
	}
}


#endif