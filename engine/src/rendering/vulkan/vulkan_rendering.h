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

extern std::vector<ImageMeta> gAllImageMetas;

struct RenderpassInfo
{
	VkRenderPass renderpass;
	VkFramebuffer framebuffer;
	VkImage targetImage;
	int defaultWidth;
	int defaultHeight;
};

extern std::vector<RenderpassInfo> gAllRenderPasses;

#endif