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

void LoadXR_Library()
{
	gLibHandle = os::LoadSharedLibrary(os::GetOpenXRLoaderLibraryName());

	xrGetInstanceProcAddr = reinterpret_cast<PFN_xrGetInstanceProcAddr>(os::GetFuncPtrImpl(gLibHandle,"xrGetInstanceProcAddr"));


	XR_LOAD_GLOBAL_FUNC(xrCreateInstance);
	XR_LOAD_GLOBAL_FUNC(xrEnumerateApiLayerProperties);
	XR_LOAD_GLOBAL_FUNC(xrEnumerateInstanceExtensionProperties);
	int a = 42;
}

void LoadXR_InstanceLevelFuncs(XrInstance instance)
{
	XR_LOAD_INSTANCE_FUNC(xrDestroyInstance);
}

void ShutdownXR_Library()
{
	os::UnloadSharedLibrary(gLibHandle);
}
#endif