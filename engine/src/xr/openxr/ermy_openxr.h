#pragma once
#include <ermy_api.h>

#ifdef ERMY_XR_OPENXR

#ifdef ERMY_OS_WINDOWS
#define XR_USE_PLATFORM_WIN32
#include <ole2.h>
#include <windows.h>
#endif

#include <vector>
#include <ermy_log.h>


#ifdef ERMY_OS_ANDROID
#define XR_USE_PLATFORM_ANDROID
#include <jni.h>
#include <android/native_activity.h>
#endif

#ifdef ERMY_GAPI_VULKAN
#define XR_USE_GRAPHICS_API_VULKAN
#include "../../rendering/vulkan/vulkan_interface.h"
#endif

#define XR_NO_PROTOTYPES
#include <openxr/openxr_platform.h>
#include <openxr/openxr.h>
#include <xr_linear.h>

void LoadXR_Library();
void ShutdownXR_Library();
void LoadXR_InstanceLevelFuncs(XrInstance instance);

extern XrInstance gXRInstance;

#define XR_DECLARE_FUNCTION( fun ) extern PFN_##fun fun;
XR_DECLARE_FUNCTION(xrResultToString);
XR_DECLARE_FUNCTION(xrGetInstanceProcAddr);
XR_DECLARE_FUNCTION(xrCreateInstance);
XR_DECLARE_FUNCTION(xrDestroyInstance);
XR_DECLARE_FUNCTION(xrEnumerateApiLayerProperties);
XR_DECLARE_FUNCTION(xrEnumerateInstanceExtensionProperties);
XR_DECLARE_FUNCTION(xrCreateDebugUtilsMessengerEXT);
XR_DECLARE_FUNCTION(xrDestroyDebugUtilsMessengerEXT);
XR_DECLARE_FUNCTION(xrGetInstanceProperties);
XR_DECLARE_FUNCTION(xrGetSystem);
XR_DECLARE_FUNCTION(xrGetSystemProperties);

XR_DECLARE_FUNCTION(xrCreateSession);
XR_DECLARE_FUNCTION(xrDestroySession);
XR_DECLARE_FUNCTION(xrBeginSession);
XR_DECLARE_FUNCTION(xrEndSession);
XR_DECLARE_FUNCTION(xrRequestExitSession);
XR_DECLARE_FUNCTION(xrPollEvent);
XR_DECLARE_FUNCTION(xrCreateReferenceSpace);
XR_DECLARE_FUNCTION(xrDestroySpace);
XR_DECLARE_FUNCTION(xrLocateSpace);
XR_DECLARE_FUNCTION(xrEnumerateReferenceSpaces);
XR_DECLARE_FUNCTION(xrEnumerateViewConfigurations);
XR_DECLARE_FUNCTION(xrGetViewConfigurationProperties);
XR_DECLARE_FUNCTION(xrEnumerateViewConfigurationViews);
XR_DECLARE_FUNCTION(xrBeginFrame);
XR_DECLARE_FUNCTION(xrEndFrame);
XR_DECLARE_FUNCTION(xrWaitFrame);
XR_DECLARE_FUNCTION(xrLocateViews);
XR_DECLARE_FUNCTION(xrEnumerateSwapchainFormats);
XR_DECLARE_FUNCTION(xrCreateSwapchain);
XR_DECLARE_FUNCTION(xrDestroySwapchain);
XR_DECLARE_FUNCTION(xrEnumerateSwapchainImages);
XR_DECLARE_FUNCTION(xrAcquireSwapchainImage);
XR_DECLARE_FUNCTION(xrWaitSwapchainImage);
XR_DECLARE_FUNCTION(xrReleaseSwapchainImage);
XR_DECLARE_FUNCTION(xrCreateActionSet);
XR_DECLARE_FUNCTION(xrDestroyActionSet);
XR_DECLARE_FUNCTION(xrCreateAction);
XR_DECLARE_FUNCTION(xrDestroyAction);
XR_DECLARE_FUNCTION(xrSuggestInteractionProfileBindings);
XR_DECLARE_FUNCTION(xrAttachSessionActionSets);
XR_DECLARE_FUNCTION(xrSyncActions);
XR_DECLARE_FUNCTION(xrGetActionStateBoolean);
XR_DECLARE_FUNCTION(xrGetActionStateFloat);
XR_DECLARE_FUNCTION(xrGetActionStateVector2f);
XR_DECLARE_FUNCTION(xrGetActionStatePose);
XR_DECLARE_FUNCTION(xrCreateActionSpace);
XR_DECLARE_FUNCTION(xrEnumerateBoundSourcesForAction);
XR_DECLARE_FUNCTION(xrGetInputSourceLocalizedName);
XR_DECLARE_FUNCTION(xrGetVulkanGraphicsRequirements2KHR);
XR_DECLARE_FUNCTION(xrGetVulkanInstanceExtensionsKHR);
XR_DECLARE_FUNCTION(xrGetVulkanDeviceExtensionsKHR);
XR_DECLARE_FUNCTION(xrGetVulkanGraphicsDevice2KHR);
XR_DECLARE_FUNCTION(xrPerfSettingsSetPerformanceLevelEXT);
XR_DECLARE_FUNCTION(xrThermalGetTemperatureTrendEXT);
XR_DECLARE_FUNCTION(xrStructureTypeToString);
XR_DECLARE_FUNCTION(xrCreateVulkanInstanceKHR);
XR_DECLARE_FUNCTION(xrCreateVulkanDeviceKHR);
XR_DECLARE_FUNCTION(xrInitializeLoaderKHR);

const char* XrResultToString(XrResult result);

#define XR_CALL(func) {XrResult res = (func); if(res < 0){ os::FatalFail( XrResultToString(res)); } }


template<typename T>
std::vector<T> EnumerateXrProps(XrResult(XRAPI_CALL* pfunc) (uint32_t cntIn, uint32_t* cntOut, T* objs), XrStructureType sType)
{
	uint32_t numProps = 0;
	XrResult result1 = pfunc(0, &numProps, nullptr);
	std::vector<T> props(numProps,{ sType });
	XrResult result = pfunc(numProps, &numProps, props.data());
	return props;
}

template<typename T>
std::vector<T> EnumerateXrProps(XrResult(XRAPI_CALL* pfunc) (const char* layerName, uint32_t cntIn, uint32_t* cntOut, T* objs), XrStructureType sType)
{
	uint32_t numProps = 0;
	XrResult result1 = pfunc(nullptr,0, &numProps, nullptr);
	std::vector<T> props(numProps, { sType });
	XrResult result = pfunc(nullptr, numProps, &numProps, props.data());
	return props;
}

struct XRInstanceExtender
{
	std::vector<const char*> enabledLayers;
	std::vector<const char*> enabledExtensions;

	std::vector<XrApiLayerProperties> all_layers;
	std::vector<XrExtensionProperties> all_extensions;
	ermy::u64 installedVersion = XR_VERSION_1_0;

	XRInstanceExtender(ermy::u64 installedVersion)
	{
		this->installedVersion = installedVersion;

		all_layers = EnumerateXrProps(xrEnumerateApiLayerProperties, XR_TYPE_API_LAYER_PROPERTIES);
		all_extensions = EnumerateXrProps(xrEnumerateInstanceExtensionProperties, XR_TYPE_EXTENSION_PROPERTIES);

		ERMY_LOG(u8"XRInstance layers lists:");

		for (auto& layer : all_layers)
		{
			ERMY_LOG(u8"XR Layer: %s description: %s", layer.layerName, layer.description);
		}

		ERMY_LOG(u8"XRInstance extension lists:");

		for (auto& ex : all_extensions)
		{
			ERMY_LOG((const char8_t*)ex.extensionName);
		}
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

	bool TryAddExtension(const char* extension_name, ermy::u64 versionInCore = XR_MAKE_VERSION(9, 9, 0))
	{
		if (versionInCore <= installedVersion)
			return true;

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

	ermy::u32 NumEnabledLayers() const
	{
		return (ermy::u32)enabledLayers.size();
	}

	ermy::u32 NumEnabledExtension() const
	{
		return (ermy::u32)enabledExtensions.size();
	}

	const char** const EnabledLayers() const
	{
		return (const char** const)enabledLayers.data();
	}

	const char** const EnabledExtensions() const
	{
		return (const char** const)enabledExtensions.data();
	}
};
#endif