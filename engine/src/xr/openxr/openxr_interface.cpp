#include "openxr_interface.h"

#ifdef ERMY_XR_OPENXR

#include "ermy_openxr.h"
#include "ermy_log.h"
#include "../../application.h"
#include <set>

#if defined(ERMY_GAPI_VULKAN)
#include "../../rendering/vulkan/vk_utils.h"
#include "../../rendering/vulkan/vulkan_rendering.h"
#endif

XrInstance gXRInstance = XR_NULL_HANDLE;
XrDebugUtilsMessengerEXT gDebugUtilsMessenger = XR_NULL_HANDLE;
XREnabledFeatures gXREnabledFeatures;
XrSystemId gXRSystemID = {};
XrSystemProperties gSystemProperties = { XR_TYPE_SYSTEM_PROPERTIES };
XrInstanceProperties gInstanceProperties{ XR_TYPE_INSTANCE_PROPERTIES };
XrSession gXRSession = XR_NULL_HANDLE; // Store session handle (could move to globals or class member)
XrSpace gReferenceSpace = XR_NULL_HANDLE;
XrViewConfigurationType gViewConfigType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
XrSwapchain gXRSwapchain_Color;
XrSwapchain gXRSwapchain_Depth;
uint32_t gSwapchainColorImageIndex = 0;
uint32_t gSwapchainDepthImageIndex = 0;
std::vector<VkImage> gSwapchainImages;
std::vector<VkImageView> gSwapchainImageViews;
std::vector<VkFramebuffer> gSwapchainFramebuffers;
VkRenderPass gXRFinalRenderPass = VK_NULL_HANDLE;
ermy::rendering::RenderPassID gErmyXRRenderPassID;

VkImage gSwapchainDepthImage = VK_NULL_HANDLE;
VmaAllocation gSwapchainDepthAllocation = {};
VkImageView gSwapchainDepthImageView = VK_NULL_HANDLE;

VkImage gSwapchainMSAAImage = VK_NULL_HANDLE;
VmaAllocation gSwapchainMSAAAllocation = {};
VkImageView gSwapchainMSAAImageView = VK_NULL_HANDLE;

VkFormat gSwapchainFormatColor = VK_FORMAT_UNDEFINED;
VkFormat gSwapchainFormatDepth = VK_FORMAT_UNDEFINED;
VkSampleCountFlagBits gSwapchainSamples = VK_SAMPLE_COUNT_1_BIT;
uint32_t gSwapchainWidth = 0;
uint32_t gSwapchainHeight = 0;
XrSessionState gSessionState = XR_SESSION_STATE_UNKNOWN;
bool gSessionRunning = false;
bool gApplicationRunning = true;
XrFrameState gFrameState{ XR_TYPE_FRAME_STATE };
bool gSessionIsActive = false;
constexpr ermy::u32 gViewCount = 2;
std::vector<XrView> gXRViews(gViewCount, { XR_TYPE_VIEW });

VkRenderPass gFinalXRPass = VK_NULL_HANDLE;

XrBool32 DebugMessageCallback(
	XrDebugUtilsMessageSeverityFlagsEXT severity,
	XrDebugUtilsMessageTypeFlagsEXT type,
	const XrDebugUtilsMessengerCallbackDataEXT* callbackData,
	void* userData) {

	if (severity >= XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		os::FatalFail(callbackData->message);
	}
	ERMY_LOG("OpenXR Debug Message: %s", callbackData->message);
	return XR_FALSE;
}

void CreateDebugUtilsMessenger(XrInstance instance) {
	XrDebugUtilsMessengerCreateInfoEXT debugCreateInfo{ XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
	debugCreateInfo.messageSeverities = XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugCreateInfo.messageTypes = XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugCreateInfo.userCallback = DebugMessageCallback;

	XR_CALL(xrCreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, &gDebugUtilsMessenger));
}

using namespace ermy;
struct EnabledExtensions
{
	bool debugUtils : 1 = false;
} gEnabledExtensions;

void CreateInstance()
{
	const auto& xrConfig = GetApplication().staticConfig.xr;

	XRInstanceExtender instance_extender(XR_VERSION_1_0);

	if (xrConfig.useDebugLayers)
	{
		gXREnabledFeatures.validationLayers = instance_extender.TryAddLayer("XR_APILAYER_LUNARG_core_validation");
	}

	gXREnabledFeatures.debugUtils = instance_extender.TryAddExtension(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);


#ifdef ERMY_GAPI_VULKAN
	gXREnabledFeatures.vulkan2 = instance_extender.TryAddExtension(XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME);
	if (!gXREnabledFeatures.vulkan2)
	{
		gXREnabledFeatures.vulkan = instance_extender.TryAddExtension(XR_KHR_VULKAN_ENABLE_EXTENSION_NAME);
	}
#endif

	//create instance
	XrInstanceCreateInfo createInfo{ XR_TYPE_INSTANCE_CREATE_INFO };
	createInfo.applicationInfo = { "", 1, "Ermy", 1, XR_API_VERSION_1_0 };
#ifdef _WIN32
	strcpy_s(createInfo.applicationInfo.applicationName, GetApplication().staticConfig.appName.c_str());
#else
	strcpy(createInfo.applicationInfo.applicationName, GetApplication().staticConfig.appName.c_str());
#endif
	createInfo.enabledApiLayerCount = (u32)instance_extender.enabledLayers.size();
	createInfo.enabledApiLayerNames = instance_extender.enabledLayers.data();
	createInfo.enabledExtensionCount = (u32)instance_extender.enabledExtensions.size();
	createInfo.enabledExtensionNames = instance_extender.enabledExtensions.data();

	XR_CALL(xrCreateInstance(&createInfo, &gXRInstance));

	LoadXR_InstanceLevelFuncs(gXRInstance);
}

void PopulateInstanceProperties()
{
	XR_CALL(xrGetInstanceProperties(gXRInstance, &gInstanceProperties));

	ERMY_LOG("OpenXR Runtime: %s Version %d.%d.%d", gInstanceProperties.runtimeName
		, XR_VERSION_MAJOR(gInstanceProperties.runtimeVersion)
		, XR_VERSION_MINOR(gInstanceProperties.runtimeVersion)
		, XR_VERSION_PATCH(gInstanceProperties.runtimeVersion));
}

void GetSystem()
{
	XrSystemGetInfo systemGI{ XR_TYPE_SYSTEM_GET_INFO };
	systemGI.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
	auto getSystemResult = xrGetSystem(gXRInstance, &systemGI, &gXRSystemID);

	if (getSystemResult == XR_ERROR_FORM_FACTOR_UNAVAILABLE || getSystemResult == XR_ERROR_FORM_FACTOR_UNSUPPORTED)
	{
		ERMY_ERROR("Has requested XR mode, but headset not found or not supported!");
		return;
	}

	// Get the System's properties for some general information about the hardware and the vendor.
	XR_CALL(xrGetSystemProperties(gXRInstance, gXRSystemID, &gSystemProperties));

	if (gXREnabledFeatures.vulkan2)
	{
		XrGraphicsRequirementsVulkan2KHR graphicsRequirements{ XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN2_KHR };

		XrResult result = xrGetVulkanGraphicsRequirements2KHR(gXRInstance, gXRSystemID, &graphicsRequirements);
		if (XR_FAILED(result))
		{
			ERMY_ERROR("Failed to get Vulkan graphics requirements: %d", result);
			return;
		}
	}
}

#ifdef ERMY_OS_ANDROID
extern ANativeActivity* gMainNativeActivity;
#endif

void xr_interface::Initialize()
{
	ERMY_LOG("Initialize OpenXR");
	LoadXR_Library();

#ifdef ERMY_OS_ANDROID
	XrLoaderInitInfoAndroidKHR loaderInitInfo = { XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR };
	loaderInitInfo.applicationVM = gMainNativeActivity->vm;
	loaderInitInfo.applicationContext = gMainNativeActivity->clazz;
	XR_CALL(xrInitializeLoaderKHR((XrLoaderInitInfoBaseHeaderKHR*)&loaderInitInfo));
#endif

	CreateInstance();

	if (gXREnabledFeatures.debugUtils)
	{
		CreateDebugUtilsMessenger(gXRInstance);
	}

	PopulateInstanceProperties();
	GetSystem();
}

void xr_interface::Shutdown()
{
	if (gDebugUtilsMessenger != XR_NULL_HANDLE) {
		xrDestroyDebugUtilsMessengerEXT(gDebugUtilsMessenger);
	}

	if (gXRInstance != XR_NULL_HANDLE) {
		xrDestroyInstance(gXRInstance);
		gXRInstance = XR_NULL_HANDLE;
	}

	ERMY_LOG("Shutdown OpenXR");
	ShutdownXR_Library();
}

void xr_interface::WaitFrame()
{
	if (!gApplicationRunning || gXRSession == XR_NULL_HANDLE || gReferenceSpace == XR_NULL_HANDLE)
	{
		ERMY_ERROR("Cannot process: Session or reference space not initialized");
		return;
	}

	XrEventDataBuffer eventData{ XR_TYPE_EVENT_DATA_BUFFER };

	// Poll OpenXR for new events
	auto XrPollEvents = [&]() -> bool {
		eventData.type = XR_TYPE_EVENT_DATA_BUFFER;
		return xrPollEvent(gXRInstance, &eventData) == XR_SUCCESS;
	};

	while (XrPollEvents())
	{
		switch (static_cast<XrStructureType>(eventData.type))
		{
		case XR_TYPE_EVENT_DATA_EVENTS_LOST:
		{
			XrEventDataEventsLost* eventsLost = reinterpret_cast<XrEventDataEventsLost*>(&eventData);
			ERMY_LOG("OPENXR: Events Lost: %u", eventsLost->lostEventCount);
			break;
		}
		case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
		{
			XrEventDataInstanceLossPending* instanceLossPending = reinterpret_cast<XrEventDataInstanceLossPending*>(&eventData);
			ERMY_LOG("OPENXR: Instance Loss Pending at: %llu", instanceLossPending->lossTime);
			gApplicationRunning = false;
			break;
		}
		case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
		{
			XrEventDataInteractionProfileChanged* interactionProfileChanged = reinterpret_cast<XrEventDataInteractionProfileChanged*>(&eventData);
			ERMY_LOG("OPENXR: Interaction Profile changed for Session: %p", interactionProfileChanged->session);
			if (interactionProfileChanged->session != gXRSession)
			{
				ERMY_LOG("XrEventDataInteractionProfileChanged for unknown Session");
				break;
			}
			break;
		}
		case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING:
		{
			XrEventDataReferenceSpaceChangePending* referenceSpaceChangePending = reinterpret_cast<XrEventDataReferenceSpaceChangePending*>(&eventData);
			ERMY_LOG("OPENXR: Reference Space Change pending for Session: %p", referenceSpaceChangePending->session);
			if (referenceSpaceChangePending->session != gXRSession)
			{
				ERMY_LOG("XrEventDataReferenceSpaceChangePending for unknown Session");
				break;
			}
			break;
		}
		case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
		{
			XrEventDataSessionStateChanged* sessionStateChanged = reinterpret_cast<XrEventDataSessionStateChanged*>(&eventData);
			if (sessionStateChanged->session != gXRSession)
			{
				ERMY_LOG("XrEventDataSessionStateChanged for unknown Session");
				break;
			}

			gSessionState = sessionStateChanged->state;

			if (gSessionState == XR_SESSION_STATE_READY)
			{
				// Session is ready. Begin the session with the primary stereo view configuration
				XrSessionBeginInfo sessionBeginInfo{ XR_TYPE_SESSION_BEGIN_INFO };
				sessionBeginInfo.primaryViewConfigurationType = gViewConfigType;
				XrResult result = xrBeginSession(gXRSession, &sessionBeginInfo);
				if (XR_FAILED(result))
				{
					ERMY_ERROR("Failed to begin session: %d", result);
					gApplicationRunning = false;
					return;
				}
				gSessionRunning = true;
				ERMY_LOG("Session started");
			}
			else if (gSessionState == XR_SESSION_STATE_STOPPING)
			{
				// Session is stopping. End the session
				XrResult result = xrEndSession(gXRSession);
				if (XR_FAILED(result))
				{
					ERMY_ERROR("Failed to end session: %d", result);
				}
				gSessionRunning = false;
				ERMY_LOG("Session stopped");
			}
			else if (gSessionState == XR_SESSION_STATE_EXITING)
			{
				// Session is exiting. Mark the application to stop
				gSessionRunning = false;
				gApplicationRunning = false;
				ERMY_LOG("Session exiting");
			}
			else if (gSessionState == XR_SESSION_STATE_LOSS_PENDING)
			{
				// Session loss is pending. Exit the application
				gSessionRunning = false;
				gApplicationRunning = false;
				ERMY_LOG("Session loss pending");
			}
			break;
		}
		default:
			break;
		}
	}

	if (gSessionRunning)
	{
		// Wait for frame		

		XrFrameWaitInfo frameWaitInfo{ XR_TYPE_FRAME_WAIT_INFO };
		XR_CALL(xrWaitFrame(gXRSession, &frameWaitInfo, &gFrameState));

		// Begin frame
		XrFrameBeginInfo frameBeginInfo{ XR_TYPE_FRAME_BEGIN_INFO };
		XR_CALL(xrBeginFrame(gXRSession, &frameBeginInfo));

		// Locate views	
		XrViewState viewState{ XR_TYPE_VIEW_STATE };
		XrViewLocateInfo viewLocateInfo{ XR_TYPE_VIEW_LOCATE_INFO };
		viewLocateInfo.viewConfigurationType = gViewConfigType;
		viewLocateInfo.displayTime = gFrameState.predictedDisplayTime;
		viewLocateInfo.space = gReferenceSpace;
		uint32_t viewCount = gViewCount;
		auto result = xrLocateViews(gXRSession, &viewLocateInfo, &viewState, gViewCount, &viewCount, gXRViews.data());
		if (XR_FAILED(result) || !(viewState.viewStateFlags & XR_VIEW_STATE_POSITION_VALID_BIT) || !(viewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT))
		{
			ERMY_ERROR("Failed to locate views or views invalid: %d", result);
			xrEndFrame(gXRSession, nullptr);
			return;
		}
	}

	gSessionIsActive = (gSessionState == XR_SESSION_STATE_SYNCHRONIZED || gSessionState == XR_SESSION_STATE_VISIBLE || gSessionState == XR_SESSION_STATE_FOCUSED);

}

void xr_interface::AcquireImage()
{
	if (gSessionRunning)
	{
		// Acquire and render to swapchains (black screen for now)
		// Acquire swapchain image
		XrSwapchainImageAcquireInfo acquireInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };

		XR_CALL(xrAcquireSwapchainImage(gXRSwapchain_Color, &acquireInfo, &gSwapchainColorImageIndex));

		XrSwapchainImageWaitInfo waitInfo{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
		waitInfo.timeout = 0;// XR_INFINITE_DURATION; // Wait indefinitely, or adjust timeout as needed (e.g., 1000000000 for 1 second)

		// Wait for swapchain image (optional but recommended)
		XR_CALL(xrWaitSwapchainImage(gXRSwapchain_Color, &waitInfo));
	}
}

void xr_interface::ReleaseImage()
{
	if (gSessionRunning)
	{
		XrSwapchainImageReleaseInfo releaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
		XR_CALL(xrReleaseSwapchainImage(gXRSwapchain_Color, &releaseInfo));
	}
}

void xr_interface::SubmitXRFrame()
{
	if (gSessionRunning)
	{
		XrCompositionLayerProjection layer{ XR_TYPE_COMPOSITION_LAYER_PROJECTION };
		XrCompositionLayerProjectionView projectionViews[2] = { { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW }, { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW } };
		layer.layerFlags = 0;
		layer.space = gReferenceSpace;

		for (uint32_t i = 0; i < gViewCount; ++i)
		{
			projectionViews[i].pose = gXRViews[i].pose;
			projectionViews[i].fov = gXRViews[i].fov;
			projectionViews[i].subImage.swapchain = gXRSwapchain_Color;
			projectionViews[i].subImage.imageRect.offset.x = 0;
			projectionViews[i].subImage.imageRect.offset.y = 0;
			projectionViews[i].subImage.imageRect.extent.width = gSwapchainWidth;
			projectionViews[i].subImage.imageRect.extent.height = gSwapchainHeight;
			projectionViews[i].subImage.imageArrayIndex = i;
		}

		layer.viewCount = gViewCount;
		layer.views = projectionViews;

		auto pLayer = &layer;

		XrFrameEndInfo frameEndInfo{ XR_TYPE_FRAME_END_INFO };
		frameEndInfo.displayTime = gFrameState.predictedDisplayTime;
		frameEndInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
		frameEndInfo.layerCount = gFrameState.shouldRender ? 1 : 0;
		frameEndInfo.layers = (const XrCompositionLayerBaseHeader* const*)&pLayer;

		XR_CALL(xrEndFrame(gXRSession, &frameEndInfo));
	}
}

bool xr_interface::BeginXRFinalRenderPass(ermy::rendering::CommandList& cl)
{
	if (gSessionIsActive && gFrameState.shouldRender)
	{
		VkCommandBuffer cbuff = (VkCommandBuffer)cl.GetNativeHandle();
		VkClearValue clearValues[3] = {};
		int colorAttachmentIndex = 0;

		if (gSwapchainFormatDepth != VK_FORMAT_UNDEFINED)
		{
			clearValues[0].depthStencil.depth = 1.0f;
			clearValues[0].depthStencil.stencil = 0;
			colorAttachmentIndex++;
		}

		clearValues[colorAttachmentIndex].color.float32[0] = 0.4f;
		clearValues[colorAttachmentIndex].color.float32[1] = 0.3f;
		clearValues[colorAttachmentIndex].color.float32[2] = 0.11f;
		clearValues[colorAttachmentIndex].color.float32[3] = 1.0f;

		colorAttachmentIndex++;

		VkRenderPassBeginInfo rpass{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		rpass.clearValueCount = colorAttachmentIndex;
		rpass.pClearValues = clearValues;
		rpass.framebuffer = gSwapchainFramebuffers[gSwapchainColorImageIndex];
		rpass.renderArea = { 0,0, gSwapchainWidth, gSwapchainHeight };
		rpass.renderPass = gXRFinalRenderPass;

		VkSubpassBeginInfo subpassBeginInfo{ VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO };
		subpassBeginInfo.contents = VK_SUBPASS_CONTENTS_INLINE;

		vkCmdBeginRenderPass2(cbuff, &rpass, &subpassBeginInfo);

		std::array<VkViewport, 2> viewports;
		std::array<VkRect2D, 2> scissors;

		for (int i = 0; i < 2; ++i)
		{
			viewports[i] = { 0,0,(float)gSwapchainWidth,(float)gSwapchainHeight,0,1 };
			scissors[i] = { 0,0,gSwapchainWidth ,gSwapchainHeight };
		}

		vkCmdSetViewport(cbuff, 0, 1, viewports.data());
		vkCmdSetScissor(cbuff, 0, 1, scissors.data());
		return true;
	}

	return false;
}

void xr_interface::EndXRFinalRenderPass(ermy::rendering::CommandList& cl)
{
	if (gSessionIsActive && gFrameState.shouldRender)
	{
		VkCommandBuffer cbuff = (VkCommandBuffer)cl.GetNativeHandle();

		VkSubpassEndInfo subpassEndInfo = { VK_STRUCTURE_TYPE_SUBPASS_END_INFO };

		vkCmdEndRenderPass2(cbuff,&subpassEndInfo);
	}
}

void InitializeSwapchains()
{
	// Enumerate view configuration views to get resolution
	uint32_t viewCount = 0;
	XrResult result = xrEnumerateViewConfigurationViews(gXRInstance, gXRSystemID, gViewConfigType, 0, &viewCount, nullptr);
	if (XR_FAILED(result))
	{
		ERMY_ERROR("Failed to enumerate view configuration views: %d", result);
		return;
	}

	std::vector<XrViewConfigurationView> configViews(viewCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
	result = xrEnumerateViewConfigurationViews(gXRInstance, gXRSystemID, gViewConfigType, viewCount, &viewCount, configViews.data());
	if (XR_FAILED(result))
	{
		ERMY_ERROR("Failed to enumerate view configuration views: %d", result);
		return;
	}

	// Assume stereo (two views, one per eye)
	if (viewCount != 2)
	{
		ERMY_ERROR("Expected 2 views for stereo, got %u", viewCount);
		return;
	}

	gSwapchainWidth = configViews[0].recommendedImageRectWidth;
	gSwapchainHeight = configViews[0].recommendedImageRectHeight;

	// Enumerate swapchain formats
	uint32_t formatCount = 0;
	result = xrEnumerateSwapchainFormats(gXRSession, 0, &formatCount, nullptr);
	if (XR_FAILED(result))
	{
		ERMY_ERROR("Failed to enumerate swapchain formats: %d", result);
		return;
	}

	std::vector<int64_t> formats(formatCount);
	result = xrEnumerateSwapchainFormats(gXRSession, formatCount, &formatCount, formats.data());
	if (XR_FAILED(result))
	{
		ERMY_ERROR("Failed to enumerate swapchain formats: %d", result);
		return;
	}

	// Find a compatible Vulkan format (e.g., RGBA8)
	for (int64_t format : formats)
	{
		VkFormat vkformat = (VkFormat)format;

		printf("%d", vkformat);
	}

	gSwapchainFormatColor = VK_FORMAT_R8G8B8A8_SRGB;// static_cast<VkFormat>(formats[0]);

	if (gSwapchainFormatColor == VK_FORMAT_UNDEFINED)
	{
		ERMY_ERROR("No compatible swapchain format found");
		return;
	}

	// Create swapchains for each view (two for stereo)

	XrSwapchainCreateInfo swapchainCreateInfo{ XR_TYPE_SWAPCHAIN_CREATE_INFO };
	swapchainCreateInfo.arraySize = 2;
	swapchainCreateInfo.format = gSwapchainFormatColor;
	swapchainCreateInfo.width = gSwapchainWidth;
	swapchainCreateInfo.height = gSwapchainHeight;
	swapchainCreateInfo.mipCount = 1;
	swapchainCreateInfo.faceCount = 1;
	swapchainCreateInfo.sampleCount = 1;
	swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;

	result = xrCreateSwapchain(gXRSession, &swapchainCreateInfo, &gXRSwapchain_Color);
	if (XR_FAILED(result))
	{
		return;
	}

	// Get swapchain images
	gSwapchainImages.clear();
	gSwapchainColorImageIndex = UINT32_MAX;
	uint32_t imageCount = 0;
	result = xrEnumerateSwapchainImages(gXRSwapchain_Color, 0, &imageCount, nullptr);
	if (XR_FAILED(result))
	{
		ERMY_ERROR("Failed to enumerate swapchain images for swapchain");
		return;
	}

	std::vector<XrSwapchainImageVulkanKHR> swapchainImages(imageCount, { XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR });
	result = xrEnumerateSwapchainImages(gXRSwapchain_Color, imageCount, &imageCount, reinterpret_cast<XrSwapchainImageBaseHeader*>(swapchainImages.data()));
	if (XR_FAILED(result))
	{
		ERMY_ERROR("Failed to enumerate swapchain images for swapchain");
		return;
	}

	for (const auto& image : swapchainImages)
	{
		gSwapchainImages.push_back(image.image);
	}

	extern int gNumberOfSwapchainFrames;
	gNumberOfSwapchainFrames = imageCount;

	VkRenderPassCreateInfo2 renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2 };

	std::vector<VkAttachmentDescription2> attachments;
	std::vector<VkAttachmentReference2> attachmentRefs;
	int depthAttachmentIndex = -1;
	int msaaAttachmentIndex = -1;
	int finalAttachmentIndex = 0;

	gSwapchainFormatDepth = VK_FORMAT_D32_SFLOAT;

	if (gSwapchainFormatDepth != VK_FORMAT_UNDEFINED)
	{
		VkAttachmentDescription2 depthAttachment{VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2};
		
		depthAttachment.format = gSwapchainFormatDepth;
		depthAttachment.samples = gSwapchainSamples;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		depthAttachmentIndex = attachments.size();
		attachments.push_back(depthAttachment);
	}

	if (gSwapchainSamples != VK_SAMPLE_COUNT_1_BIT)
	{
		VkAttachmentDescription2 msaaColorAttachment{ VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2 };
		msaaColorAttachment.format = gSwapchainFormatColor;
		msaaColorAttachment.samples = gSwapchainSamples;
		msaaColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		msaaColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		msaaColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		msaaColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		msaaColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		msaaColorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		msaaAttachmentIndex = attachments.size();
		attachments.push_back(msaaColorAttachment);
	}

	{
		VkAttachmentDescription2 colorAttachment{ VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2 };
		colorAttachment.format = gSwapchainFormatColor;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		finalAttachmentIndex = attachments.size();
		attachments.push_back(colorAttachment);
	}

	VkAttachmentReference2 finalAttachmentRef{ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2};
	finalAttachmentRef.attachment = finalAttachmentIndex;
	finalAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference2 msaaColorAttachmentRef{ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2 };
	msaaColorAttachmentRef.attachment = msaaAttachmentIndex;
	msaaColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference2 depthAttachmentRef{ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2 };
	depthAttachmentRef.attachment = depthAttachmentIndex;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription2 subpass{VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.viewMask = 0x3;

	if (depthAttachmentIndex >= 0)
	{
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
	}

	if (msaaAttachmentIndex >= 0)
	{
		subpass.pColorAttachments = &msaaColorAttachmentRef;
		subpass.pResolveAttachments = &finalAttachmentRef;
	}
	else
	{
		subpass.pColorAttachments = &finalAttachmentRef;
	}

	// These dependencies ensure proper synchronization
	std::array<VkSubpassDependency2, 2> dependencies;
	//

	dependencies[0] = {
		.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
	};
	dependencies[1] = {
		.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
		.srcSubpass = 0,
		.dstSubpass = VK_SUBPASS_EXTERNAL,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
		.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
	};

	// Define the correlation mask for stereo views (2 views: left and right eye)
	uint32_t viewMask = 0x3; // Bitmask: 0b11 (both views 0 and 1 are correlated)
	std::vector<uint32_t> correlatedViewMasks = { viewMask }; // Single subpass, so one mask

	renderPassInfo.attachmentCount = (u32)attachments.size();
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = static_cast<u32>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	renderPassInfo.correlatedViewMaskCount = static_cast<uint32_t>(correlatedViewMasks.size());
	renderPassInfo.pCorrelatedViewMasks = correlatedViewMasks.data();

	vkCreateRenderPass2(gVKDevice, &renderPassInfo, nullptr, &gXRFinalRenderPass);

	vk_utils::debug::SetName(gXRFinalRenderPass, "Final XR RenderPass");

	if (gSwapchainFormatDepth != VK_FORMAT_UNDEFINED)
	{
		VkImageCreateInfo depthImageInfo{};
		depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
		depthImageInfo.format = gSwapchainFormatDepth; // e.g., VK_FORMAT_D32_SFLOAT
		depthImageInfo.extent = { gSwapchainWidth, gSwapchainHeight, 1 };
		depthImageInfo.mipLevels = 1;
		depthImageInfo.arrayLayers = 2;
		depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		depthImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo depthAllocInfo{};
		depthAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		vmaCreateImage(gVMA_Allocator, &depthImageInfo, &depthAllocInfo,
			&gSwapchainDepthImage, &gSwapchainDepthAllocation, nullptr);
		vk_utils::debug::SetName(gSwapchainDepthImage, "Swapchain XR Depth Image");

		VkImageViewCreateInfo depthViewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		depthViewInfo.image = gSwapchainDepthImage;
		depthViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		depthViewInfo.format = gSwapchainFormatDepth;
		depthViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		depthViewInfo.subresourceRange.baseMipLevel = 0;
		depthViewInfo.subresourceRange.levelCount = 1;
		depthViewInfo.subresourceRange.baseArrayLayer = 0;
		depthViewInfo.subresourceRange.layerCount = 2;

		vkCreateImageView(gVKDevice, &depthViewInfo, nullptr, &gSwapchainDepthImageView);
		vk_utils::debug::SetName(gSwapchainDepthImageView, "Swapchain XR Depth ImageView");
	}

	// MSAA Image Creation with VMA
	if (gSwapchainSamples != VK_SAMPLE_COUNT_1_BIT)
	{
		VkImageCreateInfo msaaImageInfo{};
		msaaImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		msaaImageInfo.imageType = VK_IMAGE_TYPE_2D;
		msaaImageInfo.format = gSwapchainFormatColor;
		msaaImageInfo.extent = { gSwapchainWidth, gSwapchainHeight, 1 };
		msaaImageInfo.mipLevels = 1;
		msaaImageInfo.arrayLayers = 2;
		msaaImageInfo.samples = gSwapchainSamples; // e.g., VK_SAMPLE_COUNT_4_BIT
		msaaImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		msaaImageInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		msaaImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo msaaAllocInfo{};
		msaaAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		vmaCreateImage(gVMA_Allocator, &msaaImageInfo, &msaaAllocInfo,
			&gSwapchainMSAAImage, &gSwapchainMSAAAllocation, nullptr);
		vk_utils::debug::SetName(gSwapchainMSAAImage, "Swapchain XR MSAA Image");

		VkImageViewCreateInfo msaaViewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		msaaViewInfo.image = gSwapchainMSAAImage;
		msaaViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		msaaViewInfo.format = gSwapchainFormatColor;
		msaaViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		msaaViewInfo.subresourceRange.baseMipLevel = 0;
		msaaViewInfo.subresourceRange.levelCount = 1;
		msaaViewInfo.subresourceRange.baseArrayLayer = 0;
		msaaViewInfo.subresourceRange.layerCount = 2;

		vkCreateImageView(gVKDevice, &msaaViewInfo, nullptr, &gSwapchainMSAAImageView);
		vk_utils::debug::SetName(gSwapchainMSAAImageView, "Swapchain XR ImageView");
	}

	gSwapchainImageViews.resize(imageCount);
	gSwapchainFramebuffers.resize(imageCount);

	for (int i = 0; i < imageCount; ++i)
	{
		vk_utils::debug::SetName(gSwapchainImages[i], "Swapchain XR Target (%d)", i);

		VkImageViewCreateInfo imageViewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		imageViewInfo.image = gSwapchainImages[i];
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		imageViewInfo.format = gSwapchainFormatColor;
		imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = 2;

		vkCreateImageView(gVKDevice, &imageViewInfo, nullptr, &gSwapchainImageViews[i]);
		vk_utils::debug::SetName(gSwapchainImageViews[i], "Swapchain XR Target View (%d)", i);

		{
			VkImageView allViews[3] = { VK_NULL_HANDLE };
			int currentViewIndex = 0;

			if (gSwapchainFormatDepth != VK_FORMAT_UNDEFINED)
			{
				allViews[currentViewIndex] = gSwapchainDepthImageView;
				currentViewIndex++;
			}

			if (gSwapchainSamples != VK_SAMPLE_COUNT_1_BIT)
			{
				allViews[currentViewIndex] = gSwapchainMSAAImageView;
				currentViewIndex++;
			}

			allViews[currentViewIndex] = gSwapchainImageViews[i];
			currentViewIndex++;

			VkFramebufferCreateInfo framebufferInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
			framebufferInfo.renderPass = gXRFinalRenderPass;
			framebufferInfo.attachmentCount = currentViewIndex;
			framebufferInfo.pAttachments = allViews;
			framebufferInfo.width = gSwapchainWidth;
			framebufferInfo.height = gSwapchainHeight;
			framebufferInfo.layers = 1;

			vkCreateFramebuffer(gVKDevice, &framebufferInfo, nullptr, &gSwapchainFramebuffers[i]);

			vk_utils::debug::SetName(gSwapchainFramebuffers[i], "Swapchain XR Target Framebuffer (%d)", i);
		}
	}

	gErmyXRRenderPassID = (rendering::RenderPassID)gAllRenderPasses.size();

	RenderpassInfo rpinfo;
	rpinfo.defaultWidth = gSwapchainWidth;
	rpinfo.defaultHeight = gSwapchainHeight;
	rpinfo.framebuffer = gSwapchainFramebuffers[0];
	rpinfo.targetImage = gSwapchainImages[0];
	rpinfo.samples = gSwapchainSamples;
	rpinfo.useDepth = (gSwapchainFormatDepth != VK_FORMAT_UNDEFINED);
	rpinfo.renderpass = gXRFinalRenderPass;
	gAllRenderPasses.push_back(rpinfo);
	//gErmyXRRenderPassID = gAllRe
	ERMY_LOG("Successfully initialized swapchains for stereo rendering");
}

void xr_interface::CreateSession()
{
	if (gXRSystemID == XR_NULL_SYSTEM_ID)
	{
		ERMY_ERROR("Cannot create session: No valid XR system available");
		return;
	}

	XrSessionCreateInfo sessionCreateInfo{ XR_TYPE_SESSION_CREATE_INFO };
	sessionCreateInfo.systemId = gXRSystemID;
	sessionCreateInfo.next = nullptr;

	XrGraphicsBindingVulkan2KHR graphicsBinding{ XR_TYPE_GRAPHICS_BINDING_VULKAN2_KHR };

#ifdef ERMY_GAPI_VULKAN
	// Ensure Vulkan2 is enabled before using it
	if (gXREnabledFeatures.vulkan2)
	{

		graphicsBinding.instance = gVKInstance;
		graphicsBinding.physicalDevice = gVKPhysicalDevice;
		graphicsBinding.device = gVKDevice;
		graphicsBinding.queueFamilyIndex = gVKMainQueueFamily;
		graphicsBinding.queueIndex = 0; // Assuming first queue in family

		sessionCreateInfo.next = &graphicsBinding;
	}
	else
	{
		ERMY_ERROR("Vulkan Enable2 extension is not enabled, cannot create session with Vulkan2 graphics binding");
		return;
	}
#else
	ERMY_ERROR("Vulkan is not enabled, cannot create session with graphics binding");
	return;
#endif

	// Create the session

	XrResult result = xrCreateSession(gXRInstance, &sessionCreateInfo, &gXRSession);
	if (XR_FAILED(result))
	{
		ERMY_ERROR("Failed to create XR session: %d", result);
		return;
	}


	XrReferenceSpaceCreateInfo referenceSpaceCreateInfo{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
	referenceSpaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE;
	referenceSpaceCreateInfo.poseInReferenceSpace.orientation.w = 1.0f; // Identity orientation
	XR_CALL(xrCreateReferenceSpace(gXRSession, &referenceSpaceCreateInfo, &gReferenceSpace));


	uint32_t viewConfigTypeCount = 0;
	XR_CALL(xrEnumerateViewConfigurations(gXRInstance, gXRSystemID, 0, &viewConfigTypeCount, nullptr));

	std::vector<XrViewConfigurationType> viewConfigTypes(viewConfigTypeCount);
	XR_CALL(xrEnumerateViewConfigurations(gXRInstance, gXRSystemID, viewConfigTypeCount, &viewConfigTypeCount, viewConfigTypes.data()));

	// Find primary stereo configuration
	for (const auto& config : viewConfigTypes)
	{
		if (config == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO)
		{
			gViewConfigType = config;
			break;
		}
	}

	if (gViewConfigType == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO)
	{
		ERMY_LOG("Using primary stereo view configuration");
	}
	else
	{
		ERMY_ERROR("Primary stereo view configuration not supported");
		return;
	}

	// Initialize swapchains (called here for simplicity, but can be in a separate method)
	InitializeSwapchains();
}

VkResult openXRCreateVulkanInstance(const VkInstanceCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkInstance* pInstance)
{
	if (gXRSystemID == XR_NULL_SYSTEM_ID)
		return vkCreateInstance(pCreateInfo, pAllocator, pInstance);

	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	XrVulkanInstanceCreateInfoKHR cinfo = { XR_TYPE_VULKAN_INSTANCE_CREATE_INFO_KHR };
	cinfo.systemId = gXRSystemID;
	cinfo.pfnGetInstanceProcAddr = vkGetInstanceProcAddr;
	cinfo.vulkanCreateInfo = pCreateInfo;
	cinfo.vulkanAllocator = pAllocator;

	XR_CALL(xrCreateVulkanInstanceKHR(gXRInstance, &cinfo, pInstance, &result));

	return result;
}

VkResult openXRGetPhysicalDevice(VkPhysicalDevice* pPhysicalDevice)
{
	if (gXRSystemID == XR_NULL_SYSTEM_ID)
		return VK_ERROR_LAYER_NOT_PRESENT;

	XrVulkanGraphicsDeviceGetInfoKHR cinfo = { XR_TYPE_VULKAN_GRAPHICS_DEVICE_GET_INFO_KHR };

	cinfo.systemId = gXRSystemID;
	cinfo.vulkanInstance = gVKInstance;

	XrResult result = xrGetVulkanGraphicsDevice2KHR(gXRInstance, &cinfo, pPhysicalDevice);

	return XR_SUCCEEDED(result) ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED;
}

VkResult openXRCreateVulkanDevice(VkPhysicalDevice physicalDevice,
	const VkDeviceCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDevice* pDevice)
{
	if (gXRSystemID == XR_NULL_SYSTEM_ID)
		return vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);

	VkResult result = VK_ERROR_INITIALIZATION_FAILED;

	XrVulkanDeviceCreateInfoKHR cinfo = { XR_TYPE_VULKAN_DEVICE_CREATE_INFO_KHR };
	cinfo.pfnGetInstanceProcAddr = vkGetInstanceProcAddr;
	cinfo.systemId = gXRSystemID;
	cinfo.vulkanAllocator = pAllocator;
	cinfo.vulkanPhysicalDevice = gVKPhysicalDevice;
	cinfo.vulkanCreateInfo = pCreateInfo;

	XR_CALL(xrCreateVulkanDeviceKHR(gXRInstance, &cinfo, pDevice, &result));
	return result;
}

#endif