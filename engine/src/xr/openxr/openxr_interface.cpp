#include "openxr_interface.h"

#ifdef ERMY_XR_OPENXR

#include "ermy_openxr.h"
#include "ermy_log.h"
#include "../../application.h"
#include <set>

XrInstance gXRInstance = XR_NULL_HANDLE;

using namespace ermy;
struct EnabledExtensions
{
	bool debugUtils : 1 = false;
} gEnabledExtensions;

void xr_interface::Initialize()
{
	const auto& xrConfig = GetApplication().staticConfig.xr;

	ERMY_LOG("Initialize OpenXR");
	LoadXR_Library();


	auto availableLayers = EnumerateXrProps(xrEnumerateApiLayerProperties,XR_TYPE_API_LAYER_PROPERTIES);
	auto availableExtensions = EnumerateXrProps(xrEnumerateInstanceExtensionProperties, XR_TYPE_EXTENSION_PROPERTIES);

	bool hasCoreValidationLayer = false;

	for (auto& l : availableLayers)
	{
		ERMY_LOG("XR Layer: %s", l.layerName);

		if (strcmp(l.layerName, "XR_APILAYER_LUNARG_core_validation") == 0)
			hasCoreValidationLayer = true;
	}

	std::vector<const char*> enabledLayers;
	std::vector<const char*> enabledExtensions;

	if (xrConfig.useDebugLayers && hasCoreValidationLayer)
	{
		enabledLayers.push_back("XR_APILAYER_LUNARG_core_validation");
	}

	ERMY_LOG("XR Extensions:");
	std::set<std::string> availableExtensionsName;

	for (auto& e : availableExtensions)
	{
		ERMY_LOG("	: %s", e.extensionName);
		availableExtensionsName.insert(e.extensionName);
	}

	if (availableExtensionsName.find(XR_EXT_DEBUG_UTILS_EXTENSION_NAME) != availableExtensionsName.end())
	{
		enabledExtensions.push_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);
		gEnabledExtensions.debugUtils = true;
	}

#ifdef ERMY_GAPI_VULKAN
	enabledExtensions.push_back(XR_KHR_VULKAN_ENABLE2_EXTENSION_NAME);
#endif

	//create instance
	XrInstanceCreateInfo createInfo{ XR_TYPE_INSTANCE_CREATE_INFO };
	createInfo.applicationInfo = { "TestBedXR", 1, "Ermy", 1, XR_CURRENT_API_VERSION};
	
	createInfo.enabledApiLayerCount = (u32)enabledLayers.size(); 
	createInfo.enabledApiLayerNames = enabledLayers.data();
	createInfo.enabledExtensionCount = (u32)enabledExtensions.size();
	createInfo.enabledExtensionNames = enabledExtensions.data();

	XrResult result = xrCreateInstance(&createInfo, &gXRInstance);

	LoadXR_InstanceLevelFuncs(gXRInstance);
}

void xr_interface::Shutdown()
{
	ERMY_LOG("Shutdown OpenXR");
	ShutdownXR_Library();
}

void xr_interface::Process()
{

}

#endif