#include "vulkan_interface.h"
#include <ermy_log.h>

#include "../../graphics/canvas.h"
#include "../../application.h"
#include "../../os/os_utils.h"

VkInstance gVKInstance = VK_NULL_HANDLE;
VkAllocationCallbacks* gVKGlobalAllocationsCallbacks = nullptr;
VkDebugUtilsMessengerEXT gVKDebugMessenger = nullptr;
VkPhysicalDevice gVKPhysicalDevice = VK_NULL_HANDLE;

VkDevice gVKDevice = VK_NULL_HANDLE;
VmaAllocator gVMA_Allocator = nullptr;
VkQueue gVKMainQueue = VK_NULL_HANDLE;
VkQueue gVKComputeAsyncQueue = VK_NULL_HANDLE;
VkQueue gVKTransferAsyncQueue = VK_NULL_HANDLE;

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

	//test debug reporter instance level
	//instance_extender.enabledExtensions.push_back("grtehg54rhg45h");

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
#ifdef ERMY_OS_MACOS
	createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
	VK_CALL(vkCreateInstance(&createInfo, gVKGlobalAllocationsCallbacks, &gVKInstance));

	LoadVkInstanceLevelFuncs(gVKInstance);

	if (vkCreateDebugUtilsMessengerEXT != nullptr)
	{
		vkCreateDebugUtilsMessengerEXT(gVKInstance, &debugCreateInfo, gVKGlobalAllocationsCallbacks, &gVKDebugMessenger);
	}
}

static i8 ChoosePhysicalDeviceByHeuristics(const std::vector<VkPhysicalDevice>& allDevices)
{
	//TODO: selection heuristics
	return 0;
}

static void ChoosePhysicalDevice()
{
	const auto& renderCfg = GetApplication().staticConfig.render;
	auto phys_devices = EnumerateVulkanObjects(gVKInstance, vkEnumeratePhysicalDevices);

	for (int i = 0; i < phys_devices.size(); ++i)
	{
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(phys_devices[i], &props);

		std::string deviceType = "DISCRETE";
		if(props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		{
			deviceType = "INTEGRATED";
		}
		else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
		{
			deviceType = "VIRTUAL";
		}
		else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
		{
			deviceType = "CPU";
		}

		ERMY_LOG("Found VK device (%d): %s - (%s)", i, props.deviceName,deviceType.c_str());
	}

	auto selectedAdapterID = renderCfg.adapterID == -1 ? ChoosePhysicalDeviceByHeuristics(phys_devices) : renderCfg.adapterID;

	gVKPhysicalDevice = phys_devices[selectedAdapterID];

	if (vkGetPhysicalDeviceProperties2)
	{
		VkPhysicalDeviceDriverProperties driverProps = {};
		driverProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;

		VkPhysicalDeviceProperties2 props = {};
		props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		props.pNext = &driverProps;

		vkGetPhysicalDeviceProperties2(gVKPhysicalDevice, &props);

		auto major = VK_API_VERSION_MAJOR(props.properties.apiVersion);
		auto minor = VK_API_VERSION_MINOR(props.properties.apiVersion);
		auto patch = VK_API_VERSION_PATCH(props.properties.apiVersion);

		ERMY_LOG("Selected Physical Device: %s", props.properties.deviceName);
		ERMY_LOG("Vulkan API: %d.%d.%d", major, minor, patch);

		ERMY_LOG("DRIVER: %s %s", driverProps.driverName, driverProps.driverInfo);
	}
	else
	{
		VkPhysicalDeviceProperties props = {};
		vkGetPhysicalDeviceProperties(gVKPhysicalDevice, &props);

		auto major = VK_API_VERSION_MAJOR(props.apiVersion);
		auto minor = VK_API_VERSION_MINOR(props.apiVersion);
		auto patch = VK_API_VERSION_PATCH(props.apiVersion);

		ERMY_LOG("Selected Physical Device: %s", props.deviceName);
		ERMY_LOG("Vulkan API: %d.%d.%d", major, minor, patch);
	}

}

void CreateDevice()
{
	struct DeviceExtender
	{
		std::vector<const char*> enabledExtensions;
		std::vector<VkExtensionProperties> all_extensions;
		std::string all_extensions_str;

		DeviceExtender()
		{
			all_extensions_str.reserve(65536);

			const char* fakeLayer = nullptr;
			all_extensions = EnumerateVulkanObjects(gVKPhysicalDevice, fakeLayer, vkEnumerateDeviceExtensionProperties);

			ERMY_LOG("Device extension lists:");

			for (auto& ex : all_extensions)
			{
				ERMY_LOG(ex.extensionName);

				all_extensions_str += ex.extensionName;
				all_extensions_str += "\n";
			}
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

		u32 NumEnabledExtension() const
		{
			return (u32)enabledExtensions.size();
		}

		const char** const EnabledExtensions() const
		{
			return (const char** const)enabledExtensions.data();
		}

	} device_extender;

	std::vector<VkQueueFamilyProperties> queueFamilies(32);


	u32 familiesCount = 32;
	vkGetPhysicalDeviceQueueFamilyProperties(gVKPhysicalDevice, &familiesCount, queueFamilies.data());


	for (u32 i = 0; i < familiesCount; ++i)
	{
		VkQueueFamilyProperties queueFamily = queueFamilies[i];
		bool isGraphicsQueue = queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT;
		bool isTransferQueue = queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT;
		bool isComputeQueue = queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT;
		bool isSparseBindingQueue = queueFamily.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT;

		bool isVideoDecodeQueue = queueFamily.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR;
		bool isVideoEncodeQueue = queueFamily.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR;

		bool isOpticalFlowNVQueue = queueFamily.queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV;

		bool isProtectedQueue = queueFamily.queueFlags & VK_QUEUE_PROTECTED_BIT;

#ifdef ERMY_OS_WINDOWS
		bool supportPresent = vkGetPhysicalDeviceWin32PresentationSupportKHR != nullptr ? vkGetPhysicalDeviceWin32PresentationSupportKHR(gVKPhysicalDevice, i) : false;
#else
		bool supportPresent = false;
#endif

		ERMY_LOG("queueID: %d (%d) image gran (%d x %d x %d) %s%s%s%s%s%s%s%s",
			i, queueFamily.queueCount,
			queueFamily.minImageTransferGranularity.width,
			queueFamily.minImageTransferGranularity.height,
			queueFamily.minImageTransferGranularity.depth,
			isGraphicsQueue ? "GRAPHICS " : "",
			isTransferQueue ? "TRANSFER " : "",
			isComputeQueue ? "COMPUTE " : "",
			isSparseBindingQueue ? "SPARSE_BINDING " : "",
			isVideoDecodeQueue ? "VIDEO_DECODE " : "",
			isVideoEncodeQueue ? "VIDEO_ENCODE " : "",
			isOpticalFlowNVQueue ? "OPTICAL_FLOW_NV " : "",
			isProtectedQueue ? "PROTECTED " : "",
			supportPresent ? "PRESENT" : "");

	}

	VkPhysicalDeviceMemoryProperties memProps = {};
	vkGetPhysicalDeviceMemoryProperties(gVKPhysicalDevice, &memProps);

	for (int i = 0; i < memProps.memoryHeapCount; ++i)
	{
		ERMY_LOG("HEAP(%d) SIZE: %d MB %s", i, (memProps.memoryHeaps[i].size / (1024 * 1024)),
			memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT ? "DEVICE" : "HOST");
	}

	for (int i = 0; i < memProps.memoryTypeCount; ++i)
	{
		std::string desc = "MEM TYPE (" + std::to_string(i) + ") HEAP: " + std::to_string(memProps.memoryTypes[i].heapIndex);

		//VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT = 0x00000001,
		//	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT = 0x00000002,
		//	VK_MEMORY_PROPERTY_HOST_COHERENT_BIT = 0x00000004,
		//	VK_MEMORY_PROPERTY_HOST_CACHED_BIT = 0x00000008,
		//	VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT = 0x00000010,
		//	VK_MEMORY_PROPERTY_PROTECTED_BIT = 0x00000020,
		//	VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD = 0x00000040,
		//	VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD = 0x00000080,
		//	VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV = 0x00000100,

		if (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			desc += "| DEVICE_LOCAL_BIT";

		if (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			desc += "| HOST_VISIBLE_BIT";

		if (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
			desc += "| HOST_COHERENT_BIT";

		if (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
			desc += "| HOST_CACHED_BIT";

		if (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
			desc += "| LAZILY_ALLOCATED_BIT";

		if (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT)
			desc += "| PROTECTED_BIT";

		if (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD)
			desc += "| DEVICE_COHERENT_BIT_AMD";

		if (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD)
			desc += "| DEVICE_UNCACHED_BIT_AMD";

		if (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV)
			desc += "| RDMA_CAPABLE_BIT_NV";

		ERMY_LOG(desc.c_str());
	}


	float highPriors[2] = { 1.0f,1.0f };

	VkDeviceQueueCreateInfo queueCreateInfoGraphics;
	queueCreateInfoGraphics.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfoGraphics.pNext = 0;
	queueCreateInfoGraphics.pQueuePriorities = highPriors;
	queueCreateInfoGraphics.queueCount = 1;
	queueCreateInfoGraphics.queueFamilyIndex = 0;
	queueCreateInfoGraphics.flags = 0;

	std::vector< VkDeviceQueueCreateInfo> requestedQueues;
	requestedQueues.push_back(queueCreateInfoGraphics);

	device_extender.TryAddExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME); //TODO: fill device info with supported features

	device_extender.TryAddExtension(VK_NV_MEMORY_DECOMPRESSION_EXTENSION_NAME);
	device_extender.TryAddExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
	device_extender.TryAddExtension(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);

#ifdef ERMY_OS_MACOS
	device_extender.TryAddExtension(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif
	VkDeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(requestedQueues.size());
	deviceCreateInfo.pQueueCreateInfos = requestedQueues.data();
	deviceCreateInfo.enabledLayerCount = 0; //deprecated
	deviceCreateInfo.ppEnabledLayerNames = nullptr;
	deviceCreateInfo.pEnabledFeatures = nullptr;
	deviceCreateInfo.ppEnabledExtensionNames = device_extender.EnabledExtensions(); //deprecated
	deviceCreateInfo.enabledExtensionCount = device_extender.NumEnabledExtension();

	VkPhysicalDeviceBufferDeviceAddressFeatures pfeture = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES };
	pfeture.bufferDeviceAddress = true;

	VkPhysicalDeviceFeatures2 enabledFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	enabledFeatures.pNext = &pfeture;

	deviceCreateInfo.pNext = &enabledFeatures;
	VK_CALL(vkCreateDevice(gVKPhysicalDevice, &deviceCreateInfo, gVKGlobalAllocationsCallbacks, &gVKDevice));


	LoadVkDeviceLevelFuncs(gVKDevice);

	vkGetDeviceQueue(gVKDevice, 0, 0, &gVKMainQueue);

	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = gVKPhysicalDevice;
	allocatorInfo.device = gVKDevice;
	allocatorInfo.instance = gVKInstance;
	allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

	VmaVulkanFunctions functions;

	functions.vkAllocateMemory = vkAllocateMemory;
	functions.vkBindBufferMemory = vkBindBufferMemory;

	functions.vkBindImageMemory = vkBindImageMemory;

	functions.vkCmdCopyBuffer = vkCmdCopyBuffer;
	functions.vkCreateBuffer = vkCreateBuffer;
	functions.vkCreateImage = vkCreateImage;
	functions.vkDestroyBuffer = vkDestroyBuffer;
	functions.vkDestroyImage = vkDestroyImage;
	functions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
	functions.vkFreeMemory = vkFreeMemory;
	functions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;

	functions.vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements;
	functions.vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements;
	functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
	functions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;

	functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
	functions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
	functions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
	functions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
	functions.vkMapMemory = vkMapMemory;
	functions.vkUnmapMemory = vkUnmapMemory;

	functions.vkBindBufferMemory2KHR = vkBindBufferMemory2;
	functions.vkBindImageMemory2KHR = vkBindImageMemory2;
	functions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2;
	functions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2;
	functions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2;

	allocatorInfo.pVulkanFunctions = &functions;

	vmaCreateAllocator(&allocatorInfo, &gVMA_Allocator);

	//TODO: need to be pooled
	// 
	//VkCommandPoolCreateInfo poolInfo;
	//poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	//poolInfo.pNext = nullptr;
	//poolInfo.queueFamilyIndex = 0;
	//poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	//VK_CALL(vkCreateCommandPool(gDevice, &poolInfo, gGlobalAllocationsCallbacks, &gOneTimeSubmitCommandPool));
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