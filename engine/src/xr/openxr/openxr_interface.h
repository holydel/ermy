#pragma once
#include "../xr_interface.h"
#include "ermy_openxr.h"

#ifdef ERMY_XR_OPENXR

#ifdef ERMY_OS_WINDOWS
#define XR_USE_PLATFORM_WIN32
#include <Windows.h>
#endif

#ifdef ERMY_OS_ANDROID
#define XR_USE_PLATFORM_ANDROID
#endif

#ifdef ERMY_GAPI_VULKAN
#define XR_USE_GRAPHICS_API_VULKAN
#include "../../rendering/vulkan/ermy_vulkan.h"
#endif
#include <openxr/openxr_platform.h>

extern XrInstance gXRInstance;
#endif