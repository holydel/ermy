#include "ermy_api.h"

#if defined(ERMY_DEVICE_HEADSET)
#if defined(ERMY_XR_NONE)
#error "DEVICE HEADSET WITH NO XR PROVIDER"
#endif
#endif

//#if defined(ERMY_DEVICE_PC)
//#if !defined(ERMY_OS_WINDOWS) && !defined(ERMY_OS_LINUX) && !defined(ERMY_OS_MACOS)
//#error "PC DEVICE REQUIRES DESKTOP OS (WINDOWS, LINUX OR MACOS)"
//#endif
//#endif

#if defined(ERMY_DEVICE_MOBILE) 
#if !defined(ERMY_OS_ANDROID)
#error "MOBILE DEVICE REQUIRES MOBILE OS (ANDROID OR EMSCRIPTEN)"
#endif
#endif

#if defined(ERMY_GAPI_VULKAN)
#if defined(ERMY_OS_MACOS)
#error "VULKAN IS NOT SUPPORTED ON MACOS"
#endif
#endif

#if defined(ERMY_GAPI_METAL)
#if !defined(ERMY_OS_MACOS)
#error "METAL IS ONLY SUPPORTED ON MACOS"
#endif
#endif

#if defined(ERMY_OS_EMSCRIPTEN)
#if !defined(ERMY_GAPI_VULKAN)
#error "EMSCRIPTEN OS REQUIRES WEBGPU GRAPHICS API"
#endif
#endif