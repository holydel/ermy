#include <ermy_rendering.h>
#ifdef ERMY_GAPI_VULKAN
#include "vulkan_interface.h"
#include <vector>


extern std::vector<VkPipeline> gAllPipelines;
extern std::vector<VkPipelineLayout> gAllPipelineLayouts;
#endif