#include "../os_utils.h"
#include "win_utils.h"

void os::SetNativeThreadName(void* nativeThreadHandle, const char* utf8threadName)
{
	wchar_t outbuff[256] = {};
	os::UTF8ToWCS(utf8threadName, outbuff);

	HRESULT hr = SetThreadDescription(nativeThreadHandle, outbuff);
}

void os::UTF8ToWCS(const char* utf8string, wchar_t* outBuff, int maxBuffSize)
{
	MultiByteToWideChar(CP_UTF8, 0, utf8string, -1, outBuff, maxBuffSize);
}

void os::WCSToUTF8(const wchar_t* wcs8string, char* outBuff, int maxBuffSize)
{
	WideCharToMultiByte(CP_UTF8, 0, wcs8string, -1, outBuff, maxBuffSize, nullptr, nullptr);
}

void os::Sleep(int ms)
{
	::Sleep(ms);
}

void os::WriteDebugLogMessageIDE(const char* utf8Message)
{
	OutputDebugStringA(utf8Message);
}

void* os::LoadSharedLibrary(const char* utf8libname)
{
	return LoadLibraryA(utf8libname);
}

bool os::UnloadSharedLibrary(void* library)
{
	return FreeLibrary(static_cast<HMODULE>(library));
}

void* os::GetFuncPtrImpl(void* library, const char* funcName)
{
	return reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(library), funcName));
}

void os::FatalFail(const char* reason)
{
	ERMY_FATAL("Fatal error: %s", reason);
	DebugBreak();
}

const char* os::GetSharedLibraryFullFilename(void* libHandle)
{
	static char buffer[1024];
	GetModuleFileNameA(static_cast<HMODULE>(libHandle), buffer, sizeof(buffer));
	return buffer;
}

#ifdef EMRY_GAPI_VULKAN
const char* os::GetVulkanRuntimeLibraryName()
{
	return "vulkan-1.dll";
}
#endif