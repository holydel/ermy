
#include "android_utils.h"




void os::SetNativeThreadName(void* nativeThreadHandle, const char* utf8threadName)
{
	wchar_t outbuff[256] = {};
	os::UTF8ToWCS(utf8threadName, outbuff);

	//TODO: implement
}

void os::UTF8ToWCS(const char* utf8string, wchar_t* outBuff, int maxBuffSize)
{
	//MultiByteToWideChar(CP_UTF8, 0, utf8string, -1, outBuff, maxBuffSize);

	//TODO: implement
}

void os::WCSToUTF8(const wchar_t* wcs8string, char* outBuff, int maxBuffSize)
{
	//WideCharToMultiByte(CP_UTF8, 0, wcs8string, -1, outBuff, maxBuffSize, nullptr, nullptr);

	//TODO: implement
}

void os::Sleep(int ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void os::WriteDebugLogMessageIDE(const char* utf8Message)
{
	__android_log_print(ANDROID_LOG_DEBUG, "TAG", "%s", utf8Message);
	//TODO: research output debug string for Android Studio
	//OutputDebugStringA(utf8Message);
}
#ifdef EMRY_GAPI_VULKAN
const char* os::GetVulkanRuntimeLibraryName()
{
	return "libvulkan.so";
}
#endif