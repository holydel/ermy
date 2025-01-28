#include "vulkan_swapchain.h"
#include "../../os/os_utils.h"
#include "vulkan_interface.h"
#include <array>

using namespace ermy;

VkSwapchainKHR gVKSwapchain = VK_NULL_HANDLE;
VkSurfaceKHR gVKSurface = VK_NULL_HANDLE;
VkSurfaceCapabilitiesKHR gVKSurfaceCaps;
VkFormat gVKSurfaceFormat;
u32 gAcquiredNextImageIndex = 0;
VkSemaphore gSwapchainSemaphore = VK_NULL_HANDLE;
VkRenderPass gVKRenderPass = VK_NULL_HANDLE;
int gSwapchainCurrentFrame = 0;
int gNumberOfFrames = 3;

struct FrameInFlight
{
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkFence inFlightFence;
	VkFramebuffer framebuffer;
	VkImage image;
	VkImageView imageView;
};

std::vector<FrameInFlight> gFramesInFlight;

VkSemaphore swapchain::GetWaitSemaphores()
{ 
	return gFramesInFlight[gSwapchainCurrentFrame].imageAvailableSemaphore;
}
VkSemaphore swapchain::GetSignalSemaphores()
{ 
	return gFramesInFlight[gSwapchainCurrentFrame].renderFinishedSemaphore;
}
VkFramebuffer swapchain::GetFramebuffer()
{
	return gFramesInFlight[gAcquiredNextImageIndex].framebuffer;
}
VkRenderPass swapchain::GetRenderPass()
{
	return gVKRenderPass;
}

struct SwapchainExt
{
	//instance extensions
	bool hasSurfaceCapabilities2 : 1 = false;
	bool hasSurfaceMaintenance1 : 1 = false;	
	bool hasSwapchainColorspace : 1 = false;
	bool hasDisplay : 1 = false;
	bool hasDisplay2Props : 1 = false;
	bool hasDirectModeDisplay : 1 = false;
	bool hasProtectedCapabilities : 1 = false;
	//device extensions
	bool hasMaintenance1 : 1 = false;
	bool hasPresentId : 1 = false;
	bool hasPresentWait : 1 = false;
	bool hasGoogleTiming : 1 = false;
	bool hasExclusiveFullscreen : 1 = false;
	bool hasHDRMetadata : 1 = false;
	bool hasNVAcquireDisplay : 1 = false;
};

SwapchainExt gSwapchainEXT;
VkCommandPool gVKCommandPool = VK_NULL_HANDLE;

void createSwapchain()
{
	VkSwapchainCreateInfoKHR createInfo {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
	createInfo.clipped = VK_TRUE;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.imageArrayLayers = 1;
	createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	createInfo.imageExtent = gVKSurfaceCaps.currentExtent;
	createInfo.imageFormat = gVKSurfaceFormat;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.minImageCount = gNumberOfFrames;
	createInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	createInfo.surface = gVKSurface;
	createInfo.preTransform = gVKSurfaceCaps.currentTransform;

	vkCreateSwapchainKHR(gVKDevice, &createInfo, nullptr, &gVKSwapchain);

	u32 imageCount = 0;
	vkGetSwapchainImagesKHR(gVKDevice, gVKSwapchain, &imageCount, nullptr);
	std::vector<VkImage> images(imageCount);
	vkGetSwapchainImagesKHR(gVKDevice, gVKSwapchain, &imageCount, images.data());

	VkRenderPassCreateInfo renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = gVKSurfaceFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;	
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	// These dependencies ensure proper synchronization
	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0] = {
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
	};

	dependencies[1] = {
		.srcSubpass = 0,
		.dstSubpass = VK_SUBPASS_EXTERNAL,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.dstAccessMask = 0,
		.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
	};

	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = static_cast<u32>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	vkCreateRenderPass(gVKDevice, &renderPassInfo, nullptr, &gVKRenderPass);

	gFramesInFlight.resize(imageCount);
	for (int i = 0; i < gNumberOfFrames; ++i)
	{
		gFramesInFlight[i].image = images[i];

		VkImageViewCreateInfo imageViewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		imageViewInfo.image = images[i];
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.format = gVKSurfaceFormat;
		imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = 1;

		vkCreateImageView(gVKDevice, &imageViewInfo, nullptr, &gFramesInFlight[i].imageView);
				
		VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		vkCreateSemaphore(gVKDevice, &semaphoreInfo, nullptr, &gFramesInFlight[i].imageAvailableSemaphore);
		vkCreateSemaphore(gVKDevice, &semaphoreInfo, nullptr, &gFramesInFlight[i].renderFinishedSemaphore);

		VkFramebufferCreateInfo framebufferInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		framebufferInfo.renderPass = gVKRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &gFramesInFlight[i].imageView;
		framebufferInfo.width = gVKSurfaceCaps.currentExtent.width;
		framebufferInfo.height = gVKSurfaceCaps.currentExtent.height;
		framebufferInfo.layers = 1;

		vkCreateFramebuffer(gVKDevice, &framebufferInfo, nullptr, &gFramesInFlight[i].framebuffer);

		//VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		//fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		//VK_CALL(vkCreateFence(gVKDevice, &fenceInfo, nullptr, &gFramesInFlight[i].inFlightFence));
	}
}

void swapchain::RequestInstanceExtensions(VKInstanceExtender& instanceExtender)
{

	#ifdef ERMY_OS_WINDOWS
		instanceExtender.TryAddExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	#endif
	#ifdef ERMY_OS_ANDROID
		instanceExtender.TryAddExtension(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
	#endif
	#ifdef ERMY_OS_LINUX
		//VK_KHR_xcb_surface
		instanceExtender.TryAddExtension("VK_KHR_xcb_surface");
	#endif
	#ifdef ERMY_OS_MACOS
		instanceExtender.TryAddExtension(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
	#endif

	gSwapchainEXT.hasProtectedCapabilities = instanceExtender.TryAddExtension(VK_KHR_SURFACE_PROTECTED_CAPABILITIES_EXTENSION_NAME);

	gSwapchainEXT.hasSurfaceCapabilities2 = instanceExtender.TryAddExtension(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
	gSwapchainEXT.hasSwapchainColorspace = instanceExtender.TryAddExtension(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);

	if (gConfig.useDisplay)
	{
		gSwapchainEXT.hasDisplay = instanceExtender.TryAddExtension(VK_KHR_DISPLAY_EXTENSION_NAME);
		gSwapchainEXT.hasDisplay2Props = instanceExtender.TryAddExtension(VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME);
		gSwapchainEXT.hasDirectModeDisplay = instanceExtender.TryAddExtension(VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME);
	}

	if (gSwapchainEXT.hasSurfaceCapabilities2)
	{
		gSwapchainEXT.hasSurfaceMaintenance1 = instanceExtender.TryAddExtension(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
	}
}

void swapchain::RequestDeviceExtensions(VKDeviceExtender& device_extender)
{
	device_extender.TryAddExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME); //TODO: fill device info with supported features

	if (gSwapchainEXT.hasSurfaceMaintenance1)
	{
		gSwapchainEXT.hasMaintenance1 = device_extender.TryAddExtension(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
	}
	
	gSwapchainEXT.hasPresentId = device_extender.TryAddExtension(VK_KHR_PRESENT_ID_EXTENSION_NAME);
	gSwapchainEXT.hasPresentWait = device_extender.TryAddExtension(VK_KHR_PRESENT_WAIT_EXTENSION_NAME);
	gSwapchainEXT.hasGoogleTiming = device_extender.TryAddExtension(VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME);
	gSwapchainEXT.hasHDRMetadata = device_extender.TryAddExtension(VK_EXT_HDR_METADATA_EXTENSION_NAME);

#ifdef ERMY_OS_WINDOWS
	if (gConfig.needExclusiveFullscreen)
	{
		gSwapchainEXT.hasExclusiveFullscreen = device_extender.TryAddExtension(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);
	}

	if (gConfig.useDisplay)
	{
		gSwapchainEXT.hasNVAcquireDisplay = device_extender.TryAddExtension(VK_NV_ACQUIRE_WINRT_DISPLAY_EXTENSION_NAME);
	}
#endif
}

void swapchain::Initialize()
{
#ifdef ERMY_OS_WINDOWS
	VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = static_cast<HWND>(os::GetNativeWindowHandle());
	createInfo.hinstance = static_cast<HINSTANCE>(os::GetAppInstanceHandle());

	VK_CALL(vkCreateWin32SurfaceKHR(gVKInstance, &createInfo, nullptr, &gVKSurface));
#endif

#ifdef ERMY_OS_ANDROID
	VkAndroidSurfaceCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,nullptr };

	createInfo.flags = 0;
	createInfo.window = static_cast<ANativeWindow*>(os::GetNativeWindowHandle());

	vkCreateAndroidSurfaceKHR(gVKInstance, &createInfo, nullptr, &gVKSurface);
#endif

#ifdef ERMY_OS_LINUX
	VkXcbSurfaceCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR };
	createInfo.window = *((xcb_window_t*)nativeWindowHandle);
	createInfo.connection = (xcb_connection_t*)mercury::platform::getAppInstanceHandle();

	vkCreateXcbSurfaceKHR(gInstance, &createInfo, gGlobalAllocationsCallbacks, &gSurface);
#endif

#ifdef ERMY_OS_MACOS
	VkMetalSurfaceCreateInfoEXT createInfo{ VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT };
	createInfo.pLayer = static_cast<CAMetalLayer*>(nativeWindowHandle);

	vkCreateMetalSurfaceEXT(gInstance, &createInfo, gGlobalAllocationsCallbacks, &gSurface);
#endif

	VkBool32 is_supported = false;

	vkGetPhysicalDeviceSurfaceSupportKHR(gVKPhysicalDevice, 0, gVKSurface, &is_supported);

	if (!is_supported)
	{
		ERMY_ERROR("Swapchain surface doesn't support!");
		return;
	}

	auto support_formats = EnumerateVulkanObjects(gVKPhysicalDevice, gVKSurface, vkGetPhysicalDeviceSurfaceFormatsKHR);
	auto support_present_modes = EnumerateVulkanObjects(gVKPhysicalDevice, gVKSurface, vkGetPhysicalDeviceSurfacePresentModesKHR);
	gVKSurfaceFormat = support_formats[0].format; //TODO: selection heuristics

	VK_CALL(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gVKPhysicalDevice, gVKSurface, &gVKSurfaceCaps));

	//gNativeExtent = gSurfaceCaps.currentExtent;
	ERMY_LOG("Supported surface alpha modes: %s %s %s %s"
		, (gVKSurfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) ? " Opaque |" : ""
		, (gVKSurfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) ? " Inherit |" : ""
		, (gVKSurfaceCaps.supportedCompositeAlpha &	VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR ) ? " Post Multiplied |" : ""
		, (gVKSurfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) ? " Pre Multiplied |" : "");
		

	const VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo2{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
												   .surface = gVKSurface };
	VkSurfaceCapabilities2KHR             capabilities2{ .sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR };
	vkGetPhysicalDeviceSurfaceCapabilities2KHR(gVKPhysicalDevice, &surfaceInfo2, &capabilities2);

	gNumberOfFrames = std::max(3u, capabilities2.surfaceCapabilities.minImageCount);
	createSwapchain();	
}

void swapchain::Shutdown()
{

}

void swapchain::Process()
{

}

void swapchain::AcquireNextImage()
{
	auto& frame = gFramesInFlight[gSwapchainCurrentFrame];

	//vkWaitForFences(gVKDevice, 1, &frame.inFlightFence, VK_TRUE, UINT64_MAX);
	//vkResetFences(gVKDevice, 1, &frame.inFlightFence);

	//frame.inFlightFence
	vkAcquireNextImageKHR(gVKDevice, gVKSwapchain, UINT64_MAX, frame.imageAvailableSemaphore, VK_NULL_HANDLE, &gAcquiredNextImageIndex);
}

void swapchain::Present()
{
	auto& frame = gFramesInFlight[gSwapchainCurrentFrame];

	// Setup the presentation info, linking the swapchain and the image index
	const VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,                               // Wait for rendering to finish
		.pWaitSemaphores = &frame.renderFinishedSemaphore,  // Synchronize presentation
		.swapchainCount = 1,                               // Swapchain to present the image
		.pSwapchains = &gVKSwapchain,                    // Pointer to the swapchain
		.pImageIndices = &gAcquiredNextImageIndex,               // Index of the image to present
	};

	//vkQueueWaitIdle(gVKMainQueue);
	vkQueuePresentKHR(gVKMainQueue, &presentInfo);

	gSwapchainCurrentFrame = (gSwapchainCurrentFrame + 1) % GetNumFrames();
}

int swapchain::GetNumFrames()
{
	return gNumberOfFrames;
}