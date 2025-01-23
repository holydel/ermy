#include "vulkan_swapchain.h"
#include "../../os/os_utils.h"
#include "vulkan_interface.h"

VkSwapchainKHR gVKSwapchain = VK_NULL_HANDLE;
VkSurfaceKHR gVKSurface = VK_NULL_HANDLE;
VkSurfaceCapabilitiesKHR gVKSurfaceCaps;

bool hasSurfaceMaintance1 = false;

bool hasMaintenance1 = false;
bool hasPresentId = false;
bool hasPresentWait = false;
bool hasGoogleTiming = false;

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


	hasSurfaceMaintance1 = instanceExtender.TryAddExtension(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);
}

void swapchain::RequestDeviceExtensions(VKDeviceExtender& device_extender)
{
	device_extender.TryAddExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME); //TODO: fill device info with supported features

	if (hasSurfaceMaintance1)
	{
		hasMaintenance1 = device_extender.TryAddExtension(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);
	}
	
	hasPresentId = device_extender.TryAddExtension(VK_KHR_PRESENT_ID_EXTENSION_NAME);
	hasPresentWait = device_extender.TryAddExtension(VK_KHR_PRESENT_WAIT_EXTENSION_NAME);
	hasGoogleTiming = device_extender.TryAddExtension(VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME);
}

void swapchain::Initialize()
{
#ifdef ERMY_OS_WINDOWS
	VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = static_cast<HWND>(os::getNativeWindowHandle());
	createInfo.hinstance = static_cast<HINSTANCE>(os::getAppInstanceHandle());

	VK_CALL(vkCreateWin32SurfaceKHR(gVKInstance, &createInfo, nullptr, &gVKSurface));
#endif

#ifdef ERMY_OS_ANDROID
	VkAndroidSurfaceCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,nullptr };

	createInfo.flags = 0;
	createInfo.window = static_cast<ANativeWindow*>(nativeWindowHandle);

	vkCreateAndroidSurfaceKHR(gInstance, &createInfo, nullptr, &gSurface);
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
	auto outputFormat = support_formats[0].format; //TODO: selection heuristics

	VK_CALL(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gVKPhysicalDevice, gVKSurface, &gVKSurfaceCaps));

	//gNativeExtent = gSurfaceCaps.currentExtent;


	int a = 42;
}

void swapchain::Shutdown()
{

}

void swapchain::Process()
{

}
