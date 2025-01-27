#pragma once
#include <ermy_api.h>

#include <imgui.h>

#ifdef ERMY_PLATFORM_WINDOWS
#include <backends/imgui_impl_win32.h>
#endif

#ifdef ERMY_PLATFOR_ANDROID
#include <backends/imgui_impl_android.h>
#endif

#ifdef ERMY_GAPI_VULKAN
#include <backends/imgui_impl_vulkan.h>
#endif

#ifdef ERMY_GAPI_D3D12
#include <backends/imgui_impl_dx12.h>
#endif
