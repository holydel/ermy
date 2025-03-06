#include "ermy_openxr.h"

#ifdef ERMY_XR_OPENXR
#include "../../os/os_utils.h"

static void* gLibHandle = nullptr;

//void* LoadDeviceFunction(VkDevice device, const char* funcName)
//{
//	void* fptr = (void*)vkGetDeviceProcAddr(device, funcName);
//
//	if (fptr == nullptr)
//	{
//		std::string fname = funcName;
//		fname += "KHR";;
//		fptr = (void*)vkGetDeviceProcAddr(device, fname.c_str());
//	}
//
//	if (fptr == nullptr)
//	{
//		std::string fname = funcName;
//		fname += "EXT";;
//		fptr = (void*)vkGetDeviceProcAddr(device, fname.c_str());
//	}
//
//	return fptr;
//}

#define XR_LOAD_GLOBAL_FUNC(func) xrGetInstanceProcAddr( XR_NULL_HANDLE, #func,(PFN_xrVoidFunction*)& func)
#define XR_LOAD_INSTANCE_FUNC(func) xrGetInstanceProcAddr( instance, #func,(PFN_xrVoidFunction*)& func)
//#define XR_LOAD_DEVICE_FUNC(func) func = (PFN_##func)LoadDeviceFunction(device,#func)

#define XR_DEFINE_FUNCTION( fun ) PFN_##fun fun = nullptr;

XR_DEFINE_FUNCTION(xrGetInstanceProcAddr);
XR_DEFINE_FUNCTION(xrCreateInstance);
XR_DEFINE_FUNCTION(xrDestroyInstance);
XR_DEFINE_FUNCTION(xrEnumerateApiLayerProperties);
XR_DEFINE_FUNCTION(xrEnumerateInstanceExtensionProperties);
XR_DEFINE_FUNCTION(xrCreateDebugUtilsMessengerEXT);
XR_DEFINE_FUNCTION(xrDestroyDebugUtilsMessengerEXT);
XR_DEFINE_FUNCTION(xrGetInstanceProperties);
XR_DEFINE_FUNCTION(xrGetSystem);
XR_DEFINE_FUNCTION(xrGetSystemProperties);
XR_DEFINE_FUNCTION(xrResultToString);
XR_DEFINE_FUNCTION(xrCreateSession);
XR_DEFINE_FUNCTION(xrDestroySession);
XR_DEFINE_FUNCTION(xrBeginSession);
XR_DEFINE_FUNCTION(xrEndSession);
XR_DEFINE_FUNCTION(xrRequestExitSession);
XR_DEFINE_FUNCTION(xrPollEvent);
XR_DEFINE_FUNCTION(xrCreateReferenceSpace);
XR_DEFINE_FUNCTION(xrDestroySpace);
XR_DEFINE_FUNCTION(xrLocateSpace);
XR_DEFINE_FUNCTION(xrEnumerateReferenceSpaces);
XR_DEFINE_FUNCTION(xrEnumerateViewConfigurations);
XR_DEFINE_FUNCTION(xrGetViewConfigurationProperties);
XR_DEFINE_FUNCTION(xrEnumerateViewConfigurationViews);
XR_DEFINE_FUNCTION(xrBeginFrame);
XR_DEFINE_FUNCTION(xrEndFrame);
XR_DEFINE_FUNCTION(xrWaitFrame);
XR_DEFINE_FUNCTION(xrLocateViews);
XR_DEFINE_FUNCTION(xrEnumerateSwapchainFormats);
XR_DEFINE_FUNCTION(xrCreateSwapchain);
XR_DEFINE_FUNCTION(xrDestroySwapchain);
XR_DEFINE_FUNCTION(xrEnumerateSwapchainImages);
XR_DEFINE_FUNCTION(xrAcquireSwapchainImage);
XR_DEFINE_FUNCTION(xrWaitSwapchainImage);
XR_DEFINE_FUNCTION(xrReleaseSwapchainImage);
XR_DEFINE_FUNCTION(xrCreateActionSet);
XR_DEFINE_FUNCTION(xrDestroyActionSet);
XR_DEFINE_FUNCTION(xrCreateAction);
XR_DEFINE_FUNCTION(xrDestroyAction);
XR_DEFINE_FUNCTION(xrSuggestInteractionProfileBindings);
XR_DEFINE_FUNCTION(xrAttachSessionActionSets);
XR_DEFINE_FUNCTION(xrSyncActions);
XR_DEFINE_FUNCTION(xrGetActionStateBoolean);
XR_DEFINE_FUNCTION(xrGetActionStateFloat);
XR_DEFINE_FUNCTION(xrGetActionStateVector2f);
XR_DEFINE_FUNCTION(xrGetActionStatePose);
XR_DEFINE_FUNCTION(xrCreateActionSpace);
XR_DEFINE_FUNCTION(xrEnumerateBoundSourcesForAction);
XR_DEFINE_FUNCTION(xrGetInputSourceLocalizedName);
XR_DEFINE_FUNCTION(xrGetVulkanGraphicsRequirements2KHR);
XR_DEFINE_FUNCTION(xrGetVulkanInstanceExtensionsKHR);
XR_DEFINE_FUNCTION(xrGetVulkanDeviceExtensionsKHR);
XR_DEFINE_FUNCTION(xrGetVulkanGraphicsDevice2KHR);
XR_DEFINE_FUNCTION(xrPerfSettingsSetPerformanceLevelEXT);
XR_DEFINE_FUNCTION(xrThermalGetTemperatureTrendEXT);
XR_DEFINE_FUNCTION(xrStructureTypeToString);
XR_DEFINE_FUNCTION(xrCreateVulkanInstanceKHR);
XR_DEFINE_FUNCTION(xrCreateVulkanDeviceKHR);
XR_DEFINE_FUNCTION(xrInitializeLoaderKHR);

const char* XrResultToString(XrResult result) {
	static char buffer[XR_MAX_RESULT_STRING_SIZE];

	if (xrResultToString)
	{
		uint32_t bufferCountOutput = 0;
		if (xrResultToString(gXRInstance, result, buffer) == XR_SUCCESS) {
			return buffer;
		}
	}

	return "Unknown XR error";
}

void LoadXR_Library()
{
	gLibHandle = os::LoadSharedLibrary(os::GetOpenXRLoaderLibraryName());

	xrGetInstanceProcAddr = reinterpret_cast<PFN_xrGetInstanceProcAddr>(os::GetFuncPtrImpl(gLibHandle,"xrGetInstanceProcAddr"));

	
	XR_LOAD_GLOBAL_FUNC(xrCreateInstance);
	XR_LOAD_GLOBAL_FUNC(xrEnumerateApiLayerProperties);
	XR_LOAD_GLOBAL_FUNC(xrEnumerateInstanceExtensionProperties);
	XR_LOAD_GLOBAL_FUNC(xrInitializeLoaderKHR);
	int a = 42;
}

void LoadXR_InstanceLevelFuncs(XrInstance instance)
{
	XR_LOAD_INSTANCE_FUNC(xrResultToString);
	XR_LOAD_INSTANCE_FUNC(xrDestroyInstance);
	XR_LOAD_INSTANCE_FUNC(xrCreateDebugUtilsMessengerEXT);
	XR_LOAD_INSTANCE_FUNC(xrDestroyDebugUtilsMessengerEXT);
	XR_LOAD_INSTANCE_FUNC(xrGetInstanceProperties);
	XR_LOAD_INSTANCE_FUNC(xrGetSystem);
	XR_LOAD_INSTANCE_FUNC(xrGetSystemProperties);
	XR_LOAD_INSTANCE_FUNC(xrCreateSession);
	XR_LOAD_INSTANCE_FUNC(xrDestroySession);
	XR_LOAD_INSTANCE_FUNC(xrBeginSession);
	XR_LOAD_INSTANCE_FUNC(xrEndSession);
	XR_LOAD_INSTANCE_FUNC(xrRequestExitSession);
	XR_LOAD_INSTANCE_FUNC(xrPollEvent);
	XR_LOAD_INSTANCE_FUNC(xrCreateReferenceSpace);
	XR_LOAD_INSTANCE_FUNC(xrDestroySpace);
	XR_LOAD_INSTANCE_FUNC(xrLocateSpace);
	XR_LOAD_INSTANCE_FUNC(xrEnumerateReferenceSpaces);
	XR_LOAD_INSTANCE_FUNC(xrEnumerateViewConfigurations);
	XR_LOAD_INSTANCE_FUNC(xrGetViewConfigurationProperties);
	XR_LOAD_INSTANCE_FUNC(xrEnumerateViewConfigurationViews);
	XR_LOAD_INSTANCE_FUNC(xrBeginFrame);
	XR_LOAD_INSTANCE_FUNC(xrEndFrame);
	XR_LOAD_INSTANCE_FUNC(xrWaitFrame);
	XR_LOAD_INSTANCE_FUNC(xrLocateViews);
	XR_LOAD_INSTANCE_FUNC(xrEnumerateSwapchainFormats);
	XR_LOAD_INSTANCE_FUNC(xrCreateSwapchain);
	XR_LOAD_INSTANCE_FUNC(xrDestroySwapchain);
	XR_LOAD_INSTANCE_FUNC(xrEnumerateSwapchainImages);
	XR_LOAD_INSTANCE_FUNC(xrAcquireSwapchainImage);
	XR_LOAD_INSTANCE_FUNC(xrWaitSwapchainImage);
	XR_LOAD_INSTANCE_FUNC(xrReleaseSwapchainImage);
	XR_LOAD_INSTANCE_FUNC(xrCreateActionSet);
	XR_LOAD_INSTANCE_FUNC(xrDestroyActionSet);
	XR_LOAD_INSTANCE_FUNC(xrCreateAction);
	XR_LOAD_INSTANCE_FUNC(xrDestroyAction);
	XR_LOAD_INSTANCE_FUNC(xrSuggestInteractionProfileBindings);
	XR_LOAD_INSTANCE_FUNC(xrAttachSessionActionSets);
	XR_LOAD_INSTANCE_FUNC(xrSyncActions);
	XR_LOAD_INSTANCE_FUNC(xrGetActionStateBoolean);
	XR_LOAD_INSTANCE_FUNC(xrGetActionStateFloat);
	XR_LOAD_INSTANCE_FUNC(xrGetActionStateVector2f);
	XR_LOAD_INSTANCE_FUNC(xrGetActionStatePose);
	XR_LOAD_INSTANCE_FUNC(xrCreateActionSpace);
	XR_LOAD_INSTANCE_FUNC(xrEnumerateBoundSourcesForAction);
	XR_LOAD_INSTANCE_FUNC(xrGetInputSourceLocalizedName);
	XR_LOAD_INSTANCE_FUNC(xrGetVulkanGraphicsRequirements2KHR);
	XR_LOAD_INSTANCE_FUNC(xrGetVulkanInstanceExtensionsKHR);
	XR_LOAD_INSTANCE_FUNC(xrGetVulkanDeviceExtensionsKHR);
	XR_LOAD_INSTANCE_FUNC(xrGetVulkanGraphicsDevice2KHR);
	XR_LOAD_INSTANCE_FUNC(xrPerfSettingsSetPerformanceLevelEXT);
	XR_LOAD_INSTANCE_FUNC(xrThermalGetTemperatureTrendEXT);
	XR_LOAD_INSTANCE_FUNC(xrStructureTypeToString);
	XR_LOAD_INSTANCE_FUNC(xrCreateVulkanInstanceKHR);
	XR_LOAD_INSTANCE_FUNC(xrCreateVulkanDeviceKHR);
}

void ShutdownXR_Library()
{
	os::UnloadSharedLibrary(gLibHandle);
}
#endif