#include "vulkan_interface.h"
#ifdef ERMY_GAPI_VULKAN

#include <ermy_log.h>

#include "../../application.h"
#include "../../os/os_utils.h"
#include "vulkan_swapchain.h"
#include <sstream>
#include "vk_utils.h"

VkInstance gVKInstance = VK_NULL_HANDLE;
VkAllocationCallbacks* gVKGlobalAllocationsCallbacks = nullptr;
VkDebugUtilsMessengerEXT gVKDebugMessenger = nullptr;
VkPhysicalDevice gVKPhysicalDevice = VK_NULL_HANDLE;

VkDevice gVKDevice = VK_NULL_HANDLE;
VmaAllocator gVMA_Allocator = nullptr;
VkQueue gVKMainQueue = VK_NULL_HANDLE;
VkQueue gVKComputeAsyncQueue = VK_NULL_HANDLE;
VkQueue gVKTransferAsyncQueue = VK_NULL_HANDLE;
VkRenderPass gVKRenderPass = VK_NULL_HANDLE;
DeviceEnabledExtensions gVKDeviceEnabledExtensions;

VkDescriptorSetLayout gImguiPreviewLayout = VK_NULL_HANDLE;
VkDescriptorSetLayout gFrameConstantsLayout = VK_NULL_HANDLE;

VkDescriptorPool gStaticDescriptorsPool = VK_NULL_HANDLE;
VkSampler gLinearSampler = VK_NULL_HANDLE;

constexpr int MAX_SINGLE_TIME_COMMAND_BUFFERS = 4;
VkCommandPool gSingleTimeCommandBuffersPool = VK_NULL_HANDLE;
std::vector<VkCommandBuffer> gAllSingleTimeCommandBuffers(MAX_SINGLE_TIME_COMMAND_BUFFERS);
std::vector<VkFence> gSingleTimeCommandBuffersInUse(MAX_SINGLE_TIME_COMMAND_BUFFERS);

using namespace ermy;
Application::StaticConfig::VKConfig gVKConfig;

constexpr u32 Ver11 = VK_MAKE_VERSION(1, 1, 0);
constexpr u32 Ver12 = VK_MAKE_VERSION(1, 2, 0);
constexpr u32 Ver13 = VK_MAKE_VERSION(1, 3, 0);
constexpr u32 Ver14 = VK_MAKE_VERSION(1, 4, 0);

u32 gPhysicalDeviceAPIVersion = VK_MAKE_VERSION(1, 0, 0);

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	if (pCallbackData->messageIdNumber == 0)
	{
		if (strcmp(pCallbackData->pMessageIdName, "Loader Message") == 0)
		{
			logger::EnqueueLogMessageRAWTagged(LogSeverity::Verbose, "VULKAN_LOADER", pCallbackData->pMessage);
			return VK_FALSE;
		}
	}
	else
	{
		if (pCallbackData->messageIdNumber == 0x7f1922d7)
		{
			logger::EnqueueLogMessageRAWTagged(LogSeverity::Verbose, "VULKAN", pCallbackData->pMessage);
			return VK_FALSE;
		}
	}

	LogSeverity severity = ermy::LogSeverity::Verbose;
	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
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
	VkBool32 truePtr{ VK_TRUE };
	VkBool32 falsePtr{ VK_FALSE };

	std::vector<const char*> debug_action{ "VK_DBG_LAYER_ACTION_DEBUG_OUTPUT" };  // "VK_DBG_LAYER_ACTION_DEBUG_OUTPUT", "VK_DBG_LAYER_ACTION_BREAK"
	std::vector<const char*> report_flags{ "warn", "perf", "error" }; //"info", "warn", "perf", "error", "debug"

	uint32_t setting_duplicate_message_limit = 32;

	std::vector<unsigned int> muteVUIds = {
		0x916108d1, //perf BestPractices-NVIDIA-ClearColor-NotCompressed
		0x675dc32e, //perf BestPractices-specialuse-extension vkCreateInstance(): Attempting to enable extension VK_EXT_debug_utils, but this ext
		0xa96ad8,  //perf  BestPractices-NVIDIA-BindMemory-NoPriorityUse vkSetDeviceMemoryPriorityEXT to provide the OS with information on which allocations should stay in memory and which should be demoted first when video memory is limited. The highest priority should be given to GPU-written resources like color attachments, depth attachments, storage images, and buffers written from the GPU. Validation Performance Warning : [BestPractices - NVIDIA - BindMemory - NoPriority] | MessageID = 0xa96ad8 | vkBindImageMemory() : [NVIDIA] Use vkSetDeviceMemoryPriorityEXT to provide the OS with information on which allocations should stay in memory and which should be demoted first when video memory is limited.The highest priority should be given to GPU - written resources like color attachments, depth attachments, storage images, and buffers written from the GPU.
		0xc91ae640, //perf  BestPractices-vkEndCommandBuffer-VtxIndexOutOfBounds vkEndCommandBuffer(): Vertex buffers was bound to VkCommandBuffer 0x123e73703d50[] but no draws had a pipeline that used the vertex buffer.
		0xc714b932, // perf [ BestPractices-NVIDIA-AllocateMemory-ReuseAllocations ] vkAllocateMemory(): [NVIDIA] Reuse memory allocations instead of releasing and reallocating. A memory allocation has been released 0.011 seconds ago, and it could have been reused in place of this allocation.
		0x7f1922d7, //perf Both GPU Assisted Validation and Normal Core Check Validation are enabled, this is not recommend as it  will be very slow. Once all errors in Core Check are solved, please disable, then only use GPU-AV for best performance.
		0x24b5c69f, //Internal Warning: Ray Query validation option was enabled, but the rayQuery feature is not enabled. [Disabling gpuav_validate_ray_query]
		//0x53c1342f, //OBS layer Validation Warning: [ BestPractices-Error-Result ] Object 0: handle = 0x1218c4eb6900, type = VK_OBJECT_TYPE_INSTANCE; | MessageID = 0x53c1342f | vkGetPhysicalDeviceImageFormatProperties2(): Returned error VK_ERROR_FORMAT_NOT_SUPPORTED.
		0x58a102d7,
		//	[0]  0x2cadaeb1900, type: 6, name : Frame Command Buffer(2)
//Validation Performance Warning : [BestPractices - vkCmdEndRenderPass - redundant - attachment - on - tile] Object 0 : handle = 0x2cadaeb1900, name = Frame Command Buffer(2), type = VK_OBJECT_TYPE_COMMAND_BUFFER; | MessageID = 0x58a102d7 | vkCmdEndRenderPass() : [Arm] [IMG] : Render pass was ended, but attachment #0 (format: VK_FORMAT_R8G8B8A8_UNORM, untouched aspects VK_IMAGE_ASPECT_COLOR_BIT) was never accessed by a pipeline or clear command.On tile - based architectures, LOAD_OP_LOAD and STORE_OP_STORE consume bandwidth and should not be part of the render pass if the attachments are not intended to be accessed.
//BestPractices - vkCmdEndRenderPass - redundant - attachment - on - tile(WARN / PERF) : msgNum: 1486947031 - Validation Performance Warning : [BestPractices - vkCmdEndRenderPass - redundant - attachment - on - tile] Object 0 : handle = 0x2cadaea6700, name = Frame Command Buffer(0), type = VK_OBJECT_TYPE_COMMAND_BUFFER; | MessageID = 0x58a102d7 | vkCmdEndRenderPass() : [Arm] [IMG] : Render pass was ended, but attachment #0 (format: VK_FORMAT_R8G8B8A8_UNORM, untouched aspects VK_IMAGE_ASPECT_COLOR_BIT) was never accessed by a pipeline or clear command.On tile - based architectures, LOAD_OP_LOAD and STORE_OP_STORE consume bandwidth and should not be part of the render pass if the attachments are not intended to be accessed.

	};
	//ATION - SETTINGS ] | MessageID = 0x7f1922d7 | Both GPU Assisted Validation and Normal Core Check Validation are enabled, this is not recommend as it  will be very slow.Once all errors in Core Check are solved, please disable, then only use GPU - AV for best performance.
	std::stringstream disabledMessages;

	void createMuteVUIDs()
	{
		disabledMessages << std::hex << std::showbase;
#ifdef _WIN32
		const char separator = ',';
#else
		const char separator = ':';
#endif
		for (int i = 0; i < muteVUIds.size(); ++i)
		{
			if (i != 0)
				disabledMessages << separator;

			disabledMessages << muteVUIds[i];
		}
	}

	std::string vuidsStr = "";
	const char* vuidsCStr = nullptr;

	VkBaseInStructure* buildPNextChain()
	{
		createMuteVUIDs();

		vuidsStr = disabledMessages.str();
		vuidsCStr = vuidsStr.c_str();

		layerSettings = std::vector<VkLayerSettingEXT>{
			{layerName, "fine_grained_locking", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
			{layerName, "validate_core", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
			{layerName, "validate_sync", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
			{layerName, "thread_safety", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
			{layerName, "check_image_layout", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
			{layerName, "check_command_buffer", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
			{layerName, "check_object_in_use", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
			{layerName, "check_query", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
			{layerName, "check_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
			{layerName, "check_shaders_caching", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
			{layerName, "unique_handles", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
			{layerName, "object_lifetime", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
			{layerName, "stateless_param", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
			{layerName, "syncval_shader_accesses_heuristic", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
			{layerName, "syncval_message_extra_properties", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
			{layerName, "syncval_message_extra_properties_pretty_print", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
			{layerName, "printf_enable", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &falsePtr},
			{layerName, "printf_verbose", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &falsePtr},
#ifdef _WIN32
			{layerName, "gpuav_enable", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
			{layerName, "gpuav_image_layout", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
			{layerName, "gpuav_shader_instrumentation", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
#else
			{layerName, "gpuav_enable", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &falsePtr},
			{layerName, "gpuav_image_layout", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &falsePtr},
			{layerName, "gpuav_shader_instrumentation", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &falsePtr},
#endif
			{layerName, "validate_best_practices", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
			{layerName, "validate_best_practices_arm", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
			{layerName, "validate_best_practices_amd", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
			{layerName, "validate_best_practices_img", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
			{layerName, "validate_best_practices_nvidia", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
			{layerName, "enable_message_limit", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &truePtr},
			{layerName, "message_id_filter", VK_LAYER_SETTING_TYPE_STRING_EXT,1, &vuidsCStr},
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
	gVKConfig = renderCfg.vkConfig;
	u32 installedVersion = 0;

	if (vkEnumerateInstanceVersion != nullptr)
	{
		VK_CALL(vkEnumerateInstanceVersion(&installedVersion));

		int major = VK_VERSION_MAJOR(installedVersion);
		int minor = VK_VERSION_MINOR(installedVersion);
		int patch = VK_VERSION_PATCH(installedVersion);

		ERMY_LOG("Found vulkan instance: %d.%d.%d", major, minor, patch);
	}

	VKInstanceExtender instance_extender(installedVersion);

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

	instance_extender.TryAddExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, Ver11);

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
		gPhysicalDeviceAPIVersion = props.properties.apiVersion;
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

		gPhysicalDeviceAPIVersion = props.apiVersion;
	}

}

struct EnabledVKFeatures
{
	VkPhysicalDeviceVulkan11Features features11 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES, nullptr };
	VkPhysicalDeviceVulkan12Features features12 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, nullptr };
	VkPhysicalDeviceVulkan13Features features13 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, nullptr };
	VkPhysicalDeviceVulkan14Features features14 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES, nullptr };

	VkPhysicalDeviceSynchronization2Features sync2Features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES, nullptr };
	VkPhysicalDeviceTimelineSemaphoreFeaturesKHR timelineSemaphoreFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR, nullptr };
	VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR, nullptr };
	VkPhysicalDeviceDynamicRenderingLocalReadFeaturesKHR dynamicRenderingFeaturesLocalRead = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES_KHR, nullptr };
	
	VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR fragmentShaderBarycentricFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR };
	VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV fragmentShaderBarycentricFeaturesNV = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_NV };

	void* BuildPChains();
	void* pchain = nullptr;

} gEnabledFeatures;

void* EnabledVKFeatures::BuildPChains()
{
	features12.timelineSemaphore = true;
	timelineSemaphoreFeatures.timelineSemaphore = true;
	sync2Features.synchronization2 = true;
	features13.synchronization2 = true;
	
	features12.vulkanMemoryModel = true;
	features12.vulkanMemoryModelDeviceScope = true;
	features12.bufferDeviceAddress = true;
	features12.uniformAndStorageBuffer8BitAccess = true;

	if (gVKConfig.useDynamicRendering)
	{
		features13.dynamicRendering = true;
		features14.dynamicRenderingLocalRead = true;

		dynamicRenderingFeatures.dynamicRendering = true;
		dynamicRenderingFeaturesLocalRead.dynamicRenderingLocalRead = true;
	}

	if (gPhysicalDeviceAPIVersion >= Ver11)
	{
		NextPChain(pchain, &features11);
	}
	if (gPhysicalDeviceAPIVersion >= Ver12)
	{
		NextPChain(pchain, &features12);
	}
	if (gPhysicalDeviceAPIVersion >= Ver13)
	{
		NextPChain(pchain, &features13);
	}
	else
	{
		if (gVKDeviceEnabledExtensions.KhrSynchronization2)
			NextPChain(pchain, &dynamicRenderingFeaturesLocalRead);

		if (gVKDeviceEnabledExtensions.KhrSynchronization2)
			NextPChain(pchain, &sync2Features);

	}

	if (gPhysicalDeviceAPIVersion >= Ver14)
	{
		NextPChain(pchain, &features14);
	}
	else
	{
		if (gVKDeviceEnabledExtensions.KhrDynamicRenderingLocalRead)
			NextPChain(pchain, &dynamicRenderingFeaturesLocalRead);
	}

	if (gVKDeviceEnabledExtensions.KhrFragmentShaderBarycentric)
	{
		fragmentShaderBarycentricFeatures.fragmentShaderBarycentric = true;
		NextPChain(pchain, &fragmentShaderBarycentricFeatures);
	}

	if (gVKDeviceEnabledExtensions.NvFragmentShaderBarycentric)
	{
		fragmentShaderBarycentricFeaturesNV.fragmentShaderBarycentric = true;
		NextPChain(pchain, &fragmentShaderBarycentricFeaturesNV);
	}
	return pchain;
}

void CreateDevice()
{
	auto& renderCfg = GetApplication().staticConfig.render;

	VKDeviceExtender device_extender(gVKPhysicalDevice, gPhysicalDeviceAPIVersion);

	auto queueFamilies = EnumerateVulkanObjects(gVKPhysicalDevice, vkGetPhysicalDeviceQueueFamilyProperties);

	//std::vector<VkQueueFamilyProperties> queueFamilies(32);


	//u32 familiesCount = 32;
	//vkGetPhysicalDeviceQueueFamilyProperties(gVKPhysicalDevice, &familiesCount, queueFamilies.data());


	for (u32 i = 0; i < queueFamilies.size(); ++i)
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

	for (u32 i = 0; i < memProps.memoryHeapCount; ++i)
	{
		ERMY_LOG("HEAP(%d) SIZE: %d MB %s", i, (memProps.memoryHeaps[i].size / (1024 * 1024)),
			memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT ? "DEVICE" : "HOST");
	}

	for (u32 i = 0; i < memProps.memoryTypeCount; ++i)
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

	gVKDeviceEnabledExtensions.NvDecompressMemory = device_extender.TryAddExtension(VK_NV_MEMORY_DECOMPRESSION_EXTENSION_NAME);
	gVKDeviceEnabledExtensions.KhrBufferDeviceAddress = device_extender.TryAddExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, Ver12);
	gVKDeviceEnabledExtensions.KhrDedicatedAllocation = device_extender.TryAddExtension(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME, Ver11);
	gVKDeviceEnabledExtensions.KhrSynchronization2 = device_extender.TryAddExtension(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, Ver13);
	gVKDeviceEnabledExtensions.KhrTimelineSemaphore = device_extender.TryAddExtension(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME, Ver12);
	gVKDeviceEnabledExtensions.ExtMemoryPriority = device_extender.TryAddExtension(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME);
	gVKDeviceEnabledExtensions.ExtPageableDeviceLocalMemory = device_extender.TryAddExtension(VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME);

	VkPhysicalDeviceFeatures enabledFeatures10 = {};
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(gVKPhysicalDevice, &supportedFeatures);

	if (gVKConfig.useDynamicRendering)
	{
		gVKDeviceEnabledExtensions.KhrDynamicRendering = device_extender.TryAddExtension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, Ver13);
		gVKDeviceEnabledExtensions.KhrDynamicRenderingLocalRead = device_extender.TryAddExtension(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME, Ver14);
	}

	if (renderCfg.enableBarycentricFS)
	{
		gVKDeviceEnabledExtensions.KhrFragmentShaderBarycentric = device_extender.TryAddExtension(VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME);
		if (!gVKDeviceEnabledExtensions.KhrFragmentShaderBarycentric)
		{
			gVKDeviceEnabledExtensions.NvFragmentShaderBarycentric = device_extender.TryAddExtension(VK_NV_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME);
		}
		
		renderCfg.enableBarycentricFS = gVKDeviceEnabledExtensions.KhrFragmentShaderBarycentric || gVKDeviceEnabledExtensions.NvFragmentShaderBarycentric;
		if(!renderCfg.enableBarycentricFS)
			ERMY_WARNING("Barycentric FS requested but not supported");
		else
			ERMY_LOG("Barycentric FS enabled!");
	}

	if (renderCfg.enableGeometryShader)
	{
		renderCfg.enableGeometryShader = enabledFeatures10.geometryShader = supportedFeatures.geometryShader;

		if (!renderCfg.enableGeometryShader)
			ERMY_WARNING("Geometry Shader requested but not supported");
		else
			ERMY_LOG("Geometry Shader enabled!");
	}

#ifdef ERMY_OS_MACOS
	device_extender.TryAddExtension(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

	enabledFeatures10.imageCubeArray = supportedFeatures.imageCubeArray;

	VkDeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = gEnabledFeatures.BuildPChains();
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(requestedQueues.size());
	deviceCreateInfo.pQueueCreateInfos = requestedQueues.data();
	deviceCreateInfo.enabledLayerCount = 0; //deprecated
	deviceCreateInfo.ppEnabledLayerNames = nullptr;
	deviceCreateInfo.pEnabledFeatures = &enabledFeatures10;
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

	vk_utils::debug::SetName(gVKDevice, "Main Device");
	vk_utils::debug::SetName(gVKPhysicalDevice, "Main Physical Device");
	vk_utils::debug::SetName(gVKInstance, "Ermy Instance");


	vkGetDeviceQueue(gVKDevice, 0, 0, &gVKMainQueue);

	vk_utils::debug::SetName(gVKMainQueue, "Main Queue");

	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = gVKPhysicalDevice;
	allocatorInfo.device = gVKDevice;
	allocatorInfo.instance = gVKInstance;

	if (gVKDeviceEnabledExtensions.KhrBufferDeviceAddress)
		allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

	if (gVKDeviceEnabledExtensions.KhrDedicatedAllocation)
		allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;

	if (gVKDeviceEnabledExtensions.ExtMemoryPriority)
		allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;


	//VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT
	//VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT
	//VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT
	//
	//VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT
	//VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT
	//VMA_ALLOCATOR_CREATE_KHR_EXTERNAL_MEMORY_WIN32_BIT
	allocatorInfo.vulkanApiVersion = gPhysicalDeviceAPIVersion;

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

	const VkCommandPoolCreateInfo cmdPoolCreateInfo{
	.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
	.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
	.queueFamilyIndex = 0, //TODO: get from device
	};

	VK_CALL(vkCreateCommandPool(gVKDevice, &cmdPoolCreateInfo, gVKGlobalAllocationsCallbacks, &gSingleTimeCommandBuffersPool));
	vk_utils::debug::SetName(gSingleTimeCommandBuffersPool, "Single Time Command Pool");


	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = gSingleTimeCommandBuffersPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,			 
			.commandBufferCount = (uint32_t)gAllSingleTimeCommandBuffers.size(),
	};

	VkFenceCreateInfo fenceCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	VK_CALL(vkAllocateCommandBuffers(gVKDevice, &commandBufferAllocateInfo, gAllSingleTimeCommandBuffers.data()));
	for (int i = 0; i < MAX_SINGLE_TIME_COMMAND_BUFFERS; ++i)
	{
		vkCreateFence(gVKDevice, &fenceCreateInfo, gVKGlobalAllocationsCallbacks, &gSingleTimeCommandBuffersInUse[i]);

		vk_utils::debug::SetName(gAllSingleTimeCommandBuffers[i], "Single Time command buffer (%d)", i);
		vk_utils::debug::SetName(gSingleTimeCommandBuffersInUse[i], "Single Time fence (%d)", i);
	}
	
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

	// Linear filtering
	samplerInfo.magFilter = VK_FILTER_LINEAR; // Magnification filter
	samplerInfo.minFilter = VK_FILTER_LINEAR; // Minification filter

	// Addressing modes
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // Wrap texture coordinates
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	// Anisotropy (optional)
	samplerInfo.anisotropyEnable = VK_FALSE; // Enable anisotropy
	samplerInfo.maxAnisotropy = 0.0f; // Maximum anisotropy level

	// Other settings
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // Border color for clamped textures
	samplerInfo.unnormalizedCoordinates = VK_FALSE; // Use normalized texture coordinates
	samplerInfo.compareEnable = VK_FALSE; // Disable comparison operations
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS; // Comparison operation (if enabled)
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // Linear mipmap filtering
	samplerInfo.mipLodBias = 0.0f; // Mipmap level-of-detail bias
	samplerInfo.minLod = 0.0f; // Minimum mipmap level
	samplerInfo.maxLod = VK_LOD_CLAMP_NONE; // Maximum mipmap level (no limit)

	VK_CALL(vkCreateSampler(gVKDevice, &samplerInfo, nullptr, &gLinearSampler));
	vk_utils::debug::SetName(gLinearSampler, "Linear Sampler");

	VkDescriptorSetLayoutBinding samplerBinding{};
	samplerBinding.binding = 0;
	samplerBinding.descriptorCount = 1;
	samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerBinding.pImmutableSamplers = nullptr;// &gLinearSampler;
	samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &samplerBinding;

	VK_CALL(vkCreateDescriptorSetLayout(gVKDevice, &layoutInfo, nullptr, &gImguiPreviewLayout));

	VkDescriptorSetLayoutBinding frameConstantsBinding{};
	frameConstantsBinding.binding = 0;
	frameConstantsBinding.descriptorCount = 1;
	frameConstantsBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	frameConstantsBinding.pImmutableSamplers = nullptr;// &gLinearSampler;
	frameConstantsBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

	VkDescriptorSetLayoutCreateInfo frameConstantsLayoutInfo{};
	frameConstantsLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	frameConstantsLayoutInfo.bindingCount = 1;
	frameConstantsLayoutInfo.pBindings = &frameConstantsBinding;

	VK_CALL(vkCreateDescriptorSetLayout(gVKDevice, &frameConstantsLayoutInfo, nullptr, &gFrameConstantsLayout));

	std::vector<VkDescriptorPoolSize> staticDescriptorsPoolSizes;
	staticDescriptorsPoolSizes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , 1024 });
	staticDescriptorsPoolSizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER , 1024 });

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = (int)staticDescriptorsPoolSizes.size();
	poolInfo.pPoolSizes = staticDescriptorsPoolSizes.data();
	poolInfo.maxSets = 4096;

	VK_CALL(vkCreateDescriptorPool(gVKDevice, &poolInfo, nullptr, &gStaticDescriptorsPool));

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
#endif

void SingleTimeCommandBuffer::Sumbit()
{
	vkEndCommandBuffer(cbuff);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cbuff;
	
	vkQueueSubmit(gVKMainQueue, 1, &submitInfo, fence);
}

SingleTimeCommandBuffer AllocateSingleTimeCommandBuffer()
{
	while (true)
	{
		for (int i = 0; i < MAX_SINGLE_TIME_COMMAND_BUFFERS; ++i)
		{
			auto bufferReady = vkGetFenceStatus(gVKDevice, gSingleTimeCommandBuffersInUse[i]);
			if (bufferReady == VK_SUCCESS)
			{
				vkResetFences(gVKDevice, 1, &gSingleTimeCommandBuffersInUse[i]);
				auto result = SingleTimeCommandBuffer();
				result.cbuff = gAllSingleTimeCommandBuffers[i];
				result.fence = gSingleTimeCommandBuffersInUse[i];

				VkCommandBufferBeginInfo beginInfo{};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
				vkResetCommandBuffer(result.cbuff, 0);
				vkBeginCommandBuffer(result.cbuff, &beginInfo);

				return result;
			}
		}

		os::Sleep(1);
	}

	return SingleTimeCommandBuffer();
}
