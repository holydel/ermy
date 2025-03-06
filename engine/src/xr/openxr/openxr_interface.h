#pragma once
#include "../xr_interface.h"
#include "ermy_openxr.h"

#ifdef ERMY_XR_OPENXR

extern XrDebugUtilsMessengerEXT gDebugUtilsMessenger;

struct XREnabledFeatures
{
	bool validationLayers : 1 = false;
	bool debugUtils : 1 = false;
	bool vulkan : 1 = false;
	bool vulkan2 : 1 = false;
};

extern XREnabledFeatures gXREnabledFeatures;

extern ermy::rendering::RenderPassID gErmyXRRenderPassID;
extern std::vector<XrView> gXRViews;

#ifdef ERMY_GAPI_VULKAN
VkResult openXRCreateVulkanInstance(const VkInstanceCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkInstance* pInstance);

VkResult openXRGetPhysicalDevice(VkPhysicalDevice* pPhysicalDevice);

VkResult openXRCreateVulkanDevice(VkPhysicalDevice physicalDevice,
	const VkDeviceCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDevice* pDevice);
#endif

#endif