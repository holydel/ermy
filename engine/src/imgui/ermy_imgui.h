#pragma once
#include <ermy_api.h>

#include <imgui.h>

#ifdef ERMY_OS_WINDOWS
#include <Windows.h>
#include <backends/imgui_impl_win32.h>
#endif

#ifdef ERMY_OS_ANDROID
#include <backends/imgui_impl_android.h>
#endif

#ifdef ERMY_GAPI_VULKAN
#include "../rendering/vulkan/ermy_vulkan.h"
#define VOLK_H_
#include <backends/imgui_impl_vulkan.h>
#endif

#ifdef ERMY_GAPI_D3D12
#include <backends/imgui_impl_dx12.h>
#endif
