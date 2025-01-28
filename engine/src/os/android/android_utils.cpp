
#include "android_utils.h"
#include <stdio.h>
#include <android/native_activity.h>
#include <android/native_window.h>
#include <android/log.h>
#include <android/input.h>

#include <android/ndk-version.h>

#include <string>
#include <thread>
#include <atomic>

#include <dlfcn.h>
#include <unordered_map>



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

void* os::LoadSharedLibrary(const char* utf8libname)
{
	return dlopen(utf8libname, RTLD_NOW);
}

bool os::UnloadSharedLibrary(void* library)
{
	dlclose(library);
	return true;
}

std::unordered_map<void*, void*> ptrToLibHandleMap;

void* os::GetFuncPtrImpl(void* library, const char* funcName)
{
	auto result = dlsym(library, funcName);

	ptrToLibHandleMap[library] = result;
	return result;
}

void os::FatalFail(const char* reason)
{
	//mercury::write_log_message("%s\n", reason);
	//DebugBreak();
}

const char* os::GetSharedLibraryFullFilename(void* libHandle)
{
	static Dl_info dl_info = {};
	dladdr(ptrToLibHandleMap[libHandle], &dl_info);

	return dl_info.dli_fname;
}

#ifdef EMRY_GAPI_VULKAN
const char* os::GetVulkanRuntimeLibraryName()
{
	return "libvulkan.so";
}
#endif