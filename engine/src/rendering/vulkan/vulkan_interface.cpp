#include "vulkan_interface.h"
#include <ermy_log.h>

#include "../../graphics/canvas.h"
#include "../../application.h"
#include "../../os/os_utils.h"
#include "vulkan_swapchain.h"

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

struct ValidationSettings
{
	VkBool32 fine_grained_locking{ VK_TRUE };
	VkBool32 validate_core{ VK_TRUE };
	VkBool32 check_image_layout{ VK_TRUE };
	VkBool32 check_command_buffer{ VK_TRUE };
	VkBool32 check_object_in_use{ VK_TRUE };
	VkBool32 check_query{ VK_TRUE };
	VkBool32 check_shaders{ VK_TRUE };
	VkBool32 check_shaders_caching{ VK_TRUE };
	VkBool32 unique_handles{ VK_TRUE };
	VkBool32 object_lifetime{ VK_TRUE };
	VkBool32 stateless_param{ VK_TRUE };
	VkBool32 setting_validate_sync = { VK_TRUE };
	VkBool32 setting_thread_safety = { VK_TRUE };

	std::vector<const char*> debug_action{ "VK_DBG_LAYER_ACTION_LOG_MSG", "VK_DBG_LAYER_ACTION_DEBUG_OUTPUT" };  // "VK_DBG_LAYER_ACTION_DEBUG_OUTPUT", "VK_DBG_LAYER_ACTION_BREAK"
	std::vector<const char*> report_flags{ "warn", "perf", "error" }; //"info", "warn", "perf", "error", "debug"

	VkBool32 setting_enable_message_limit = VK_TRUE;
	uint32_t setting_duplicate_message_limit = 32;

	VkBaseInStructure* buildPNextChain()
	{
		layerSettings = std::vector<VkLayerSettingEXT>{
			{layerName, "fine_grained_locking", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &fine_grained_locking},
			{layerName, "validate_core", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &validate_core},
			{layerName, "validate_sync", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &setting_validate_sync},
			{layerName, "thread_safety", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &setting_thread_safety},
			{layerName, "check_image_layout", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &check_image_layout},
			{layerName, "check_command_buffer", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &check_command_buffer},
			{layerName, "check_object_in_use", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &check_object_in_use},
			{layerName, "check_query", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &check_query},
			{layerName, "check_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &check_shaders},
			{layerName, "check_shaders_caching", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &check_shaders_caching},
			{layerName, "unique_handles", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &unique_handles},
			{layerName, "object_lifetime", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &object_lifetime},
			{layerName, "stateless_param", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &stateless_param},
			{layerName, "enable_message_limit", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &setting_enable_message_limit},
			{layerName, "duplicate_message_limit", VK_LAYER_SETTING_TYPE_UINT32_EXT, 1, &setting_duplicate_message_limit},
			{layerName, "debug_action", VK_LAYER_SETTING_TYPE_STRING_EXT, uint32_t(debug_action.size()), debug_action.data()},
			{layerName, "report_flags", VK_LAYER_SETTING_TYPE_STRING_EXT, uint32_t(report_flags.size()), report_flags.data()},

		};
		layerSettingsCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT,
			.settingCount = uint32_t(layerSettings.size()),
			.pSettings = layerSettings.data(),
		};

		return reinterpret_cast<VkBaseInStructure*>(&layerSettingsCreateInfo);
	}

	static constexpr const char* layerName{ "VK_LAYER_KHRONOS_validation" };
	std::vector<VkLayerSettingEXT> layerSettings{};
	VkLayerSettingsCreateInfoEXT   layerSettingsCreateInfo{};
};

static ValidationSettings validationSettings = {};

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

	VKInstanceExtender instance_extender;

	bool isDebugLayers = renderCfg.enableDebugLayers;

	if (isDebugLayers)
	{
		instance_extender.TryAddLayer("VK_LAYER_KHRONOS_validation");
		instance_extender.TryAddExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	swapchain::Configure(GetApplication().staticConfig);

	if (GetApplication().staticConfig.outputMode != Application::StaticConfig::OutputMode::Headless)
	{
		instance_extender.TryAddExtension(VK_KHR_SURFACE_EXTENSION_NAME);
		swapchain::RequestInstanceExtensions(instance_extender);
	}

	//test debug reporter instance level
	//instance_extender.enabledExtensions.push_back("grtehg54rhg45h");

	instance_extender.TryAddExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
	instance_extender.TryAddExtension(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
	instance_extender.TryAddExtension(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

#ifdef ERMY_OS_MACOS
	//Found drivers that contain devices which support the portability subset, but the instance does not enumerate portability drivers!
	//Applications that wish to enumerate portability drivers must set the VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR bit in the VkInstanceCreateInfo flags
	//and enable the VK_KHR_portability_enumeration instance extension.
	instance_extender.TryAddExtension(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

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
		debugCreateInfo.pNext = validationSettings.buildPNextChain();
		//NextChainPushFront(&createInfo, validationSettings.buildPNextChain());
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

		if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
			deviceType = "INTEGRATED";
		if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
			deviceType = "VIRTUAL";
		if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
			deviceType = "CPU";

		ERMY_LOG("Found VK device (%d): %s - (%s)", i, props.deviceName, deviceType.c_str());
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
	VKDeviceExtender device_extender(gVKPhysicalDevice);

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

	swapchain::RequestDeviceExtensions(device_extender);

	device_extender.TryAddExtension(VK_NV_MEMORY_DECOMPRESSION_EXTENSION_NAME);
	device_extender.TryAddExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
	device_extender.TryAddExtension(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
	device_extender.TryAddExtension(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
	device_extender.TryAddExtension(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);

	VkPhysicalDeviceVulkan11Features features11 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES, nullptr };
	VkPhysicalDeviceVulkan12Features features12 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, nullptr };
	VkPhysicalDeviceVulkan13Features features13 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, nullptr };
	VkPhysicalDeviceVulkan14Features features14 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES, nullptr };

	features11.pNext = &features12;

	features12.timelineSemaphore = true;

	VkPhysicalDeviceSynchronization2Features sync2Features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES, nullptr };
	sync2Features.synchronization2 = true;
	features12.pNext = &sync2Features;

#ifdef ERMY_OS_MACOS
	device_extender.TryAddExtension(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif
	VkDeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = &features11;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(requestedQueues.size());
	deviceCreateInfo.pQueueCreateInfos = requestedQueues.data();
	deviceCreateInfo.enabledLayerCount = 0; //deprecated
	deviceCreateInfo.ppEnabledLayerNames = nullptr;
	deviceCreateInfo.pEnabledFeatures = nullptr;
	deviceCreateInfo.ppEnabledExtensionNames = device_extender.EnabledExtensions(); //deprecated
	deviceCreateInfo.enabledExtensionCount = device_extender.NumEnabledExtension();

	//VkPhysicalDeviceBufferDeviceAddressFeatures pfeture = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES };
	//pfeture.bufferDeviceAddress = true;

	//VkPhysicalDeviceFeatures2 enabledFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	//
	//enabledFeatures.pNext = &pfeture;

	//deviceCreateInfo.pNext = &enabledFeatures;
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

void rendering_interface::Initialize()
{
	ERMY_LOG("VULKAN Initialize");

	LoadVK_Library();

	InitializeInstance();
	ChoosePhysicalDevice();
	CreateDevice();
	//printf("RENDERING","WEBGPU Initialize/n");
}

void rendering_interface::Shutdown()
{
	ERMY_LOG("VULKAN Shutdown");

	//printf("RENDERING","WEBGPU Shutdown/n");
}

void rendering_interface::Process()
{

}