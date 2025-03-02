#include <ermy_rendering.h>
#ifdef ERMY_GAPI_VULKAN
#include "vulkan_interface.h"
#include <vector>


extern std::vector<VkPipeline> gAllPipelines;
extern std::vector<VkPipelineLayout> gAllPipelineLayouts;
extern std::vector<VkShaderModule> gAllShaderModules;
extern std::vector<VkBuffer> gAllBuffers;
extern std::vector<VkImage> gAllImages;
extern std::vector<VkImageView> gAllImageViews;
extern std::vector<VkDescriptorSet> gAllImageDescriptors;

struct ImageMeta
{
	ermy::u16 width;
	ermy::u16 height;
	ermy::u16 depth;
	VkFormat format;
	VkSampleCountFlagBits samples;
};

struct BufferMeta
{
	ermy::u64 size = 0;
	void* presistentMappedPtr = nullptr;

	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VmaAllocation stagingBufferAllocation = VK_NULL_HANDLE;

	ermy::rendering::BufferUsage bufferUsage;
};

extern std::vector<ImageMeta> gAllImageMetas;

struct RenderpassInfo
{
	VkRenderPass renderpass = VK_NULL_HANDLE;
	VkFramebuffer framebuffer = VK_NULL_HANDLE;
	VkImage targetImage = VK_NULL_HANDLE;
	int defaultWidth = 0;
	int defaultHeight = 0;
	bool useDepth = false;
	VkSampleCountFlagBits samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
};

extern std::vector<RenderpassInfo> gAllRenderPasses;

#endif