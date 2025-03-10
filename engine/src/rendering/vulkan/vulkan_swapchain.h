#pragma once
#include "../swapchain_interface.h"

#include "ermy_vulkan.h"

extern VkSwapchainKHR gVKSwapchain;
extern VkSurfaceKHR gVKSurface;
extern VkSurfaceCapabilitiesKHR gVKSurfaceCaps;
extern VkFormat gVKSurfaceFormat;
extern VkSampleCountFlagBits gVKSurfaceSamples;
extern VkFormat gVKSurfaceDepthFormat;
//TODO: Collect information about VK_EXT_swapchain_maintenance1, VK_KHR_present_id, VK_KHR_present_wait, VK_GOOGLE_display_timing

namespace swapchain
{
	void RequestInstanceExtensions(VKInstanceExtender &instanceExtender);
	void RequestDeviceExtensions(VKDeviceExtender& device_extender);

	VkSemaphore GetWaitSemaphores();
	VkSemaphore GetSignalSemaphores();
	VkFramebuffer GetFramebuffer();
	VkRenderPass GetRenderPass();

	void InitSwapchain();
	void ShutdownSwapchain();
	void ShutdownOldSwapchain();
	void InitSwapchainResources();
	void ShutdownSwapchainResources();
	
	VkImage GetCurrentImage();
	VkImageView GetCurrentImageView();
	VkImageLayout GetCurrentImageLayout();
}
