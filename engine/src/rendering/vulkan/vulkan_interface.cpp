#include "vulkan_interface.h"
#include <ermy_log.h>

#include "../../graphics/canvas.h"
#include "../../application.h"
#include "../../os/os_utils.h"

VkInstance gVKInstance = VK_NULL_HANDLE;
VkAllocationCallbacks* gVKGlobalAllocationsCallbacks = nullptr;
VkDebugUtilsMessengerEXT gVKDebugMessenger = nullptr;

using namespace ermy;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	LogSeverity severity = ermy::LogSeverity::Verbose;
	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		severity = LogSeverity::Verbose;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		severity = LogSeverity::Verbose;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		severity = LogSeverity::Warning;
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		severity = LogSeverity::Error;
		break;
	}

	logger::EnqueueLogMessageRAWTagged(severity, "VULKAN", pCallbackData->pMessage);

	return VK_FALSE;
}

void InitializeInstance()
{
	const auto& renderCfg = GetApplication().staticConfig.render;

	u32 installedVersion = 0;

	if (vkEnumerateInstanceVersion != nullptr)
	{
		
		VK_CALL(vkEnumerateInstanceVersion(&installedVersion));

		int major = VK_VERSION_MAJOR(installedVersion);
		int minor = VK_VERSION_MINOR(installedVersion);
		int patch = VK_VERSION_PATCH(installedVersion);

		ERMY_LOG("Found vulkan instance: %d.%d.%d", major, minor, patch);
	}

	struct InstanceExtender
	{
		std::vector<const char*> enabledLayers;
		std::vector<const char*> enabledExtensions;

		std::vector<VkLayerProperties> all_layers;
		std::vector<VkExtensionProperties> all_extensions;

		InstanceExtender()
		{
			all_layers = EnumerateVulkanObjects(vkEnumerateInstanceLayerProperties);
			all_extensions = EnumerateVulkanObjects(vkEnumerateInstanceExtensionProperties);
		}

		bool TryAddLayer(const char* layer_name)
		{
			bool exists = false;

			for (auto& l : all_layers)
			{
				if (strcmp(l.layerName, layer_name) == 0)
				{
					exists = true;
					break;
				}
			}

			if (exists)
			{
				enabledLayers.push_back(layer_name);
			}

			return exists;
		}

		bool TryAddExtension(const char* extension_name)
		{
			bool exists = false;

			for (auto& e : all_extensions)
			{
				if (strcmp(e.extensionName, extension_name) == 0)
				{
					exists = true;
					break;
				}
			}

			if (exists)
			{
				enabledExtensions.push_back(extension_name);
			}

			return exists;
		}

		u32 NumEnabledLayers() const
		{
			return (u32)enabledLayers.size();
		}

		u32 NumEnabledExtension() const
		{
			return (u32)enabledExtensions.size();
		}

		const char** const EnabledLayers() const
		{
			return (const char** const)enabledLayers.data();
		}

		const char** const EnabledExtensions() const
		{
			return (const char** const)enabledExtensions.data();
		}

	} instance_extender;

	bool isDebugLayers = renderCfg.enableDebugLayers;

	if (isDebugLayers)
	{
		instance_extender.TryAddLayer("VK_LAYER_KHRONOS_validation");
		instance_extender.TryAddExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	if (GetApplication().staticConfig.outputMode != Application::StaticConfig::OutputMode::Headless)
	{
		instance_extender.TryAddExtension(VK_KHR_SURFACE_EXTENSION_NAME);
	}

	instance_extender.enabledExtensions.push_back("grtehg54rhg45h");

	instance_extender.TryAddExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

#ifdef ERMY_OS_WINDOWS
	instance_extender.TryAddExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
#ifdef ERMY_OS_ANDROID
	instance_extender.TryAddExtension(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif
#ifdef ERMY_OS_LINUX
	//VK_KHR_xcb_surface
	instance_extender.TryAddExtension("VK_KHR_xcb_surface");
#endif
#ifdef ERMY_OS_MACOS
	//Found drivers that contain devices which support the portability subset, but the instance does not enumerate portability drivers!
	//Applications that wish to enumerate portability drivers must set the VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR bit in the VkInstanceCreateInfo flags
	//and enable the VK_KHR_portability_enumeration instance extension.
	instance_extender.TryAddExtension(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
	instance_extender.TryAddExtension(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#endif

	if (isDebugLayers)	
		instance_extender.TryAddExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugCreateInfo.flags = 0;
	debugCreateInfo.pNext = nullptr;
	debugCreateInfo.pUserData = nullptr;
	debugCreateInfo.pfnUserCallback = &debugCallback;
	debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;


	VkApplicationInfo vkAppInfo;
	vkAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	vkAppInfo.pNext = nullptr;
	vkAppInfo.pApplicationName = GetApplication().staticConfig.appName.c_str();
	vkAppInfo.pEngineName = "mercury";
	vkAppInfo.applicationVersion = GetApplication().staticConfig.version.packed;
	vkAppInfo.apiVersion = installedVersion ? installedVersion : VK_MAKE_API_VERSION(0, 1, 1, 0);

	VkInstanceCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

	if (isDebugLayers)
	{
		createInfo.pNext = &debugCreateInfo;
	}

	createInfo.pApplicationInfo = &vkAppInfo;
	createInfo.ppEnabledLayerNames = instance_extender.EnabledLayers();
	createInfo.ppEnabledExtensionNames = instance_extender.EnabledExtensions();
	createInfo.enabledLayerCount = instance_extender.NumEnabledLayers();
	createInfo.enabledExtensionCount = instance_extender.NumEnabledExtension();

	createInfo.flags = 0;
#ifdef MERCURY_PLATFORM_MACOS
	createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
	VK_CALL(vkCreateInstance(&createInfo, gVKGlobalAllocationsCallbacks, &gVKInstance));

	LoadVkInstanceLevelFuncs(gVKInstance);

	if (vkCreateDebugUtilsMessengerEXT != nullptr)
	{
		vkCreateDebugUtilsMessengerEXT(gVKInstance, &debugCreateInfo, gVKGlobalAllocationsCallbacks, &gVKDebugMessenger);
	}
}

void ChoosePhysicalDevice()
{

}

void CreateDevice()
{

}

void rendering::Initialize()
{
   ERMY_LOG("VULKAN Initialize");
  
   LoadVK_Library();

   InitializeInstance();
   ChoosePhysicalDevice();
   CreateDevice();
   //printf("RENDERING","WEBGPU Initialize/n");
}

void rendering::Shutdown()
{
    ERMY_LOG("VULKAN Shutdown");

    //printf("RENDERING","WEBGPU Shutdown/n");
}

void rendering::Process()
{
  
}