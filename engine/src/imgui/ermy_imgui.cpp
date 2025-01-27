#include "ermy_imgui.h"

#ifdef ERMY_PLATFORM_WINDOWS
#include <backends/imgui_impl_win32.cpp>
#endif

#ifdef ERMY_PLATFOR_ANDROID
#include <backends/imgui_impl_android.cpp>
#endif

#ifdef ERMY_GAPI_VULKAN
#include <backends/imgui_impl_vulkan.cpp>
#endif

#ifdef ERMY_GAPI_D3D12
#include <backends/imgui_impl_dx12.cpp>
#endif