#pragma once
#include <ermy_api.h>

#ifdef ERMY_XR_OPENXR
#include <vector>
#define XR_NO_PROTOTYPES
#include <openxr/openxr.h>

void LoadXR_Library();
void ShutdownXR_Library();
void LoadXR_InstanceLevelFuncs(XrInstance instance);

#define XR_CALL(func) {VkResult res = (func); if(res < 0){ os::FatalFail( "unimplementedted" ); } }

#define XR_DECLARE_FUNCTION( fun ) extern PFN_##fun fun;

XR_DECLARE_FUNCTION(xrGetInstanceProcAddr);
XR_DECLARE_FUNCTION(xrCreateInstance);
XR_DECLARE_FUNCTION(xrDestroyInstance);
XR_DECLARE_FUNCTION(xrEnumerateApiLayerProperties);
XR_DECLARE_FUNCTION(xrEnumerateInstanceExtensionProperties);

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

#endif